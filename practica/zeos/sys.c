/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#include <c_buffer.h>

#define LECTURA 0
#define ESCRIPTURA 1

void *get_ebp();

extern circular_buffer cb;

extern struct shm_mem_struct shm_mem[10];

int check_fd(int fd, int permissions)
{
  if (fd != 1)
    return -EBADF;
  if (permissions != ESCRIPTURA)
    return -EACCES;
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
  return -ENOSYS;
}

int sys_getpid()
{
  return current()->PID;
}

int global_PID = 1000;

int ret_from_fork()
{
  return 0;
}

int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;

  /* Any free task_struct? */
  if (list_empty(&freequeue))
    return -ENOMEM;

  lhcurrent = list_first(&freequeue);

  list_del(lhcurrent);

  uchild = (union task_union *)list_head_to_task_struct(lhcurrent);

  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));

  /* new pages dir */
  allocate_DIR((struct task_struct *)uchild);

  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  for (pag = 0; pag < NUM_PAG_DATA; pag++)
  {
    new_ph_pag = alloc_frame();
    if (new_ph_pag != -1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA + pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i = 0; i < pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA + i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA + i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);

      /* Return error */
      return -EAGAIN;
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag = 0; pag < NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag = 0; pag < NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE + pag, get_frame(parent_PT, PAG_LOG_INIT_CODE + pag));
  }
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  for (pag = NUM_PAG_KERNEL + NUM_PAG_CODE; pag < NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
    phys_mem[get_frame(parent_PT, pag)]++; // da igual parent_pt o process_pt
    process_PT[pag].bits.rw = 0;
    parent_PT[pag].bits.rw = 0;
  }

  for(pag = NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA; pag < TOTAL_PAGES; pag++){
    int frame = get_frame(parent_PT, pag);
    if(frame != 0){
      set_ss_pag(process_PT, pag, frame);
      int length = sizeof(shm_mem) / sizeof(shm_mem[0]);
      for(int i = 0; i < length; ++i){
        if(shm_mem[i].shm_mem_frame_addr == frame) shm_mem[i].referencias++;
      }
    }
  }
  /* Deny access to the child's memory space */
  set_cr3(get_DIR(current()));

  uchild->task.PID = ++global_PID;
  uchild->task.state = ST_READY;

  int register_ebp; /* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int)get_ebp();
  register_ebp = (register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp = register_ebp + sizeof(DWord);

  DWord temp_ebp = *(DWord *)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp -= sizeof(DWord);
  *(DWord *)(uchild->task.register_esp) = (DWord)&ret_from_fork;
  uchild->task.register_esp -= sizeof(DWord);
  *(DWord *)(uchild->task.register_esp) = temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state = ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);

  return uchild->task.PID;
}

#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes)
{
  char localbuffer[TAM_BUFFER];
  int bytes_left;
  int ret;

  if ((ret = check_fd(fd, ESCRIPTURA)))
    return ret;
  if (nbytes < 0)
    return -EINVAL;
  if (!access_ok(VERIFY_READ, buffer, nbytes))
    return -EFAULT;

  bytes_left = nbytes;
  while (bytes_left > TAM_BUFFER)
  {
    copy_from_user(buffer, localbuffer, TAM_BUFFER);
    ret = sys_write_console(localbuffer, TAM_BUFFER);
    bytes_left -= ret;
    buffer += ret;
  }
  if (bytes_left > 0)
  {
    copy_from_user(buffer, localbuffer, bytes_left);
    ret = sys_write_console(localbuffer, bytes_left);
    bytes_left -= ret;
  }
  return (nbytes - bytes_left);
}

int sys_read(char *b, int maxchars)
{
  if (!access_ok(VERIFY_WRITE, b, maxchars)) return -EFAULT;
  if (maxchars > cb.count)
    maxchars = cb.count; // Caso en el que queramos leer mas caracteres de los actuales en el buffer
  for (int i = 0; i < maxchars; ++i)
  {
    char c_dequeue = dequeue(&cb);
    copy_to_user(&c_dequeue, &b[i], sizeof(char));
  }
  return maxchars;
}

// x = nº column,  x < 80
// y = nº row,  y < 25
int sys_gotoxy(int x, int y)
{
  if ( x >= 80 || x < 0) return -EINVAL;
  if ( y >= 25 || y < 0) return -EINVAL;
  
  set_xy(x, y);
  return 1; // Nose que se tiene que retornar, de mientras retorno un 1.
}

int sys_set_color(int fg, int bg)
{
  if(fg < 0 || fg > 15){
    return -EINVAL;
  }
  if(bg < 0 || bg > 15){
    return -EINVAL;
  }
  set_text_color(fg, bg);
  return 1;
}

void *sys_shmat(int id, void *addr)
{
  //Entonces cualquier dirección que sea diferente de 0 estara mal
  //Cuando sea 0 estara bien alineada ya que sera addr AND 0x00000FFF
  unsigned int page_align = 0xfff;  //0xfff
  unsigned int page = (int)addr & page_align;
  if (page != 0) return -ECANCELED;

  page_table_entry *page_table = get_PT(current());
  //Comprobación del param id 
  int length = sizeof(shm_mem) / sizeof(shm_mem[0]);
  if (id < 0 || id >= length) return -EINVAL;

  //Comprobación de que la addr pasada esta libre
  //En mi logica es dessplazammiento a la derecha >>
  unsigned int access = (int)addr >> 12;
  if (page_table[access].bits.present != 0)
  {
    int found = 0;
    while (access < TOTAL_PAGES)
    { 
      if (page_table[access].bits.present == 0){
        set_ss_pag(page_table, access, shm_mem[id].shm_mem_frame_addr);
        shm_mem[id].referencias++;
        return (void*) (access << 12);
      }
      ++access;
    }
    return -EINVAL;
  }
  shm_mem[id].referencias++;
  set_ss_pag(page_table, access, shm_mem[id].shm_mem_frame_addr);
  return (void*) (access << 12);
}

int sys_shmdt(void* addr){
  //Comprobación de parámetros
  //Cualquier dirección que sea diferente de 0 estara mal
  //Cuando sea 0 estara bien alineada ya que sera addr AND 0x00000FFF
  unsigned long page_align = 0xfffff000;
  unsigned long page = (unsigned long) addr & page_align;

  //Obtenemos el indice de la TP
  page_table_entry *page_table = get_PT(current());
  unsigned int access = page >> 12;
  //Caso en que la dirección dada super el indice maximo
  if (access >= TOTAL_PAGES) return -ECANCELED; 

  //Miramos en la entrada de la TP si hay traducción logica <-> fisica
  if (page_table[access].bits.present == 0) return -ECANCELED;
  int frame = get_frame(page_table, access);

    /*Comprobación de que addrs es una logica con traducción a una física
      de a la SHARED REGION!!!!!
    */
  int length = sizeof(shm_mem) / sizeof(shm_mem[0]);
  int i = 0;
  while ( i < length){
    if(frame == shm_mem[i].shm_mem_frame_addr) break;
    i++;
  }
  if(i == length) return -ECANCELED;
  shm_mem[i].referencias--;
  if(shm_mem[i].a_borrar == 0 || shm_mem[i].referencias != 0) return 0;
  //void* direccion = (void*) (shm_mem[i].shm_mem_frame_addr << 12); //Pasamos de id a dirección con el shift a la izquiera como en FORK
  for (unsigned long long * j = page; j < page + (PAGE_SIZE / sizeof(unsigned long long)); *j = 0, j++);
  shm_mem[i].a_borrar = 0;
  del_ss_pag(page_table, access);
  set_cr3(get_DIR(current()));

  return 0;
}

int sys_shmrm(int id){
  //Comprobación de parámetros
  if ( id > 10 || id < 0) return -EINVAL;
  shm_mem[id].a_borrar = 1;

  return 0;
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{
  int i;

  page_table_entry *process_PT = get_PT(current());

  // Deallocate all the propietary physical pages
  for (i = 0; i < NUM_PAG_DATA; i++)
  {
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA + i));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA + i);
  }

  for(i = NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA; i < TOTAL_PAGES; i++){
    int frame = get_frame(process_PT, i);
    if(frame != 0){
      int length = sizeof(shm_mem) / sizeof(shm_mem[0]);
      for(int i = 0; i < length; ++i){
        if(shm_mem[i].shm_mem_frame_addr == frame){
          shm_mem[i].referencias--;
          if(shm_mem[i].referencias == 0){
            del_ss_pag(process_PT, i);
          }
        }
      }
    }
  }

  /* Free task_struct */
  list_add_tail(&(current()->list), &freequeue);

  current()->PID = -1;

  /* Restarts execution of the next process */
  sched_next_rr();
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;

  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats)))
    return -EFAULT;

  if (pid < 0)
    return -EINVAL;
  for (i = 0; i < NR_TASKS; i++)
  {
    if (task[i].task.PID == pid)
    {
      task[i].task.p_stats.remaining_ticks = remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}
