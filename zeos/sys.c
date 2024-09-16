/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <libc.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern int zeos_ticks;
char sys_buff[64];
int starting_pid = 1000;

union task_union *u_child;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int ret_from_fork(){
  return 0;
}

int sys_fork()
{
  struct list_head *list_actual = NULL;
  union task_union *u_child;
  //comprovem si tenim processos en free (ens queden processos i no ens hem quedat sense)
  if(list_empty(&freequeue)) return -ENOMEM;
  //l'agafes de la llista
  list_actual = list_first(&freequeue);
  //l'elimines de la llista
	list_del(list_actual);
  //obtens el task_union
	u_child = list_head_to_task_struct(list_actual); // cambiar

  copy_data(current(), u_child, sizeof(union task_union));
  allocate_DIR((struct task_struct *)u_child);

  page_table_entry *child_PT = get_PT((struct task_struct *)u_child);

  /* Estructura on guardarem tot els frames*/
  int free_physical_frames[NUM_PAG_DATA];
  for(int i = 0; i < NUM_PAG_DATA; ++i){

    free_physical_frames[i] = alloc_frame();

    if(free_physical_frames[i] == -1){ /* Desalocatem tot el que hem alocatat */
      for(int j = 0; j < i; ++j){
        free_frame(free_physical_frames[j]);
      }
      list_add_tail(list_actual, &freequeue);

      return -EAGAIN;
    }
  }

  page_table_entry *parent_PT = get_PT(current());

  /* Es copia el contingut de la part de kernel. Se sap que esta a les primeres pàgines lògiques */
  for(int i = 0; i < NUM_PAG_KERNEL; ++i){
    set_ss_pag(child_PT, i, get_frame(parent_PT, i));
  }
  /* Es copia el contingut de code. Se sap que esta a PAG_LOG_INIT_CODE + offset */
  for(int i = 0; i < NUM_PAG_CODE; ++i){
    set_ss_pag(child_PT, PAG_LOG_INIT_CODE+i, get_frame(parent_PT, PAG_LOG_INIT_CODE+i));
  }
  /* Es copia el contingut de data als frames físics que haviem trobat abans */
  for(int i = 0; i < NUM_PAG_DATA; ++i){
    set_ss_pag(child_PT, PAG_LOG_INIT_DATA+i, free_physical_frames[i]);
  }
  //Ara toca el mapeig
  for(int i = NUM_PAG_KERNEL+NUM_PAG_CODE; i < NUM_PAG_KERNEL+NUM_PAG_DATA+NUM_PAG_CODE; i++){
    set_ss_pag(parent_PT, i+NUM_PAG_DATA, get_frame(child_PT, i)); // pilles la pagina del fill i la poses a NUM_PAG_DATA+i (no hi ha res en aquella localitzacio)
    // Pagines alineadas a 4KB
    copy_data(i << 12, ((i+NUM_PAG_DATA) << 12), PAGE_SIZE); // copies les dades
    del_ss_pag(parent_PT, i+NUM_PAG_DATA);
  }

  /* En aquest punt ja no es pot accedir a la memoria del fill */
  set_cr3(get_DIR(current()));

  u_child->task.PID = ++starting_pid;
  u_child->task.state = ST_READY;
  u_child->task.pParent = current(); // guardem el punter al pare
  u_child->task.pending_unblocks = 0;
  INIT_LIST_HEAD(&(u_child->task.childs)); // inicialitzem la llista del fill (per a que es pugui guardar els fills)
  list_add_tail(&(u_child->task.aParentList), &(current()->childs)); // afegim el fill en la nostra llista que ja estara inicialitzada per el nostre pare

  u_child->stack[KERNEL_STACK_SIZE - 19] = (unsigned long) 0;
  u_child->stack[KERNEL_STACK_SIZE - 18] = (unsigned long) &ret_from_fork;
  u_child->task.kernel_esp = (unsigned int) &(u_child->stack[KERNEL_STACK_SIZE - 19]);

  list_add_tail(&(u_child->task.list), &readyqueue);
  return u_child->task.PID;
}

void sys_exit()
{
  struct task_struct *task_destruir = current();
  page_table_entry *page_destruir = get_PT(task_destruir);

  if(task_destruir->PID != 1 && task_destruir->PID != 0){ //significa que no te pare
    list_del(&(task_destruir->aParentList));
  }

  list_add_tail(&(idle_task->childs), &task_destruir->childs);
  list_del(&task_destruir->childs);
  // no se que faltaria per actualizar la informació del pare. En principi no cal perque sobrescruiras el task struct amb el nou procés

  // no cal treure l'enllaça amb el kernel o el codi perque quan s'agafi el proxim PCB es sobrescriura.
  for(int i = NUM_PAG_KERNEL+NUM_PAG_CODE; i < NUM_PAG_CODE+NUM_PAG_DATA+NUM_PAG_KERNEL; ++i){
    free_frame(get_frame(page_destruir, i));
    del_ss_pag(page_destruir, i);
  }
  
  update_process_state_rr(task_destruir, &freequeue);
  sched_next_rr();
}

void sys_block(void){
  if(current()->pending_unblocks == 0){ //bloquejem
    current()->state = ST_BLOCKED;
    list_del(&(current()->list));
    list_add_tail(&(current()->list), &blocked);
    schedule();
  } else {
    current()->pending_unblocks -= 1;
  }
}

int sys_unblock(int pid){
  int trobat = 0;
  struct task_struct *task_found;
  struct task_struct *task_iterator;
  struct list_head *iterator;
  list_for_each(iterator, &(current()->childs)){
    task_iterator = list_head_to_task_struct(iterator);
    if(task_iterator->PID == pid){
      trobat = 1;
      task_found = task_iterator;
    }
  }
  if(trobat){ // no es fill teu
    if(task_found->state == ST_BLOCKED){
      task_found->state = ST_READY;
      list_del(&(task_found->list));
      list_add_tail(&(task_found->list), &readyqueue);
      return 0;
    } else {
      task_found->pending_unblocks += 1;
    }
  } else return -1;
}

int sys_write(int fd, char *buffer, int size){
  int retorno = check_fd(fd, ESCRIPTURA);
  if(retorno != 0) return retorno;

  if(buffer == NULL) return -EFAULT;

  if(access_ok(VERIFY_READ, buffer, size) == 0) return -EFAULT;

  if(size < 0) return -EINVAL;

  int total = size;
  int offset = 0;
  int bytes_leidos = 0;

  while(total > 0){
    if(total < 64){
      copy_from_user(&buffer[offset], &sys_buff[offset], total);
      bytes_leidos += sys_write_console(&sys_buff[offset], total);
      offset += total;
      total -= total;
    } else {
      copy_from_user(&buffer[offset], &sys_buff[offset], 64);
      bytes_leidos += sys_write_console(&sys_buff[offset], 64);
      offset += 64;
      total -= 64;
    }
  }
  if(bytes_leidos == size) return bytes_leidos;
  else return -1;
}

int sys_gettime(){
  return zeos_ticks;
}