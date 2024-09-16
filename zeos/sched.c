/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <devices.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

struct list_head freequeue;
struct list_head readyqueue;

struct task_struct *idle_task;
struct task_struct *task1_task;
int ticks_since_cpu;

int turn = 0;

void inner_task_switch_asm(unsigned int *current_kernel_esp, unsigned int new_kernel_esp);


struct task_struct *change_task(){
	printc('c');
	if(turn == 0){
		printc('a');
		turn = 1;
		return idle_task;
	} else {
		printc('b');
		turn = 0;
		return task1_task;
	}
}

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}

void inner_task_switch(union task_union *new){
	tss.esp0 = KERNEL_ESP(new); // modify system stack through int
	write_msr(0x175, (long) tss.esp0); // modify system stack through sysenter

	set_cr3(get_DIR(&new->task)); // change page table a new
	return inner_task_switch_asm(&(((struct task_struct *)current())->kernel_esp), ((struct task_struct *)new)->kernel_esp);
}

int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void)
{
	// COJO UNO Y LUEGO LE ASSIGNO VALOR A TODOS LOS CAMPOS DEL UNION TASK_UNION DEL FREEQUEUE
	// LE DOY UNA TABLA DE PAGINAS Y LUEGO LO PREPARO PARA QUE SEA EJECUTADO
	struct list_head *list_actual = list_first(&freequeue);
	list_del(list_actual);
	struct task_struct *actual = list_head_to_task_struct(list_actual); // cambiar
	actual->PID = 0;
	allocate_DIR(actual);
	actual->quantum = QUANTUM;
	actual->pParent = NULL;
	INIT_LIST_HEAD(&(actual->childs));
	
	((union task_union*)actual) -> stack[KERNEL_STACK_SIZE - 1] = (unsigned long) cpu_idle; // @ret 
	((union task_union*)actual) -> stack[KERNEL_STACK_SIZE - 2] = (unsigned long) 0; // ebp
	actual->kernel_esp = &(((union task_union*)actual) -> stack[KERNEL_STACK_SIZE - 2]);

	idle_task = actual;

}

void init_task1(void)
{
	struct list_head *list_actual = list_first(&freequeue);
	list_del(list_actual);
	struct task_struct *actual = list_head_to_task_struct(list_actual); // cambiar
	actual->PID = 1;
	actual->quantum = QUANTUM;
	allocate_DIR(actual);
	set_user_pages(actual);
	actual->pParent = NULL; // no te pare
	INIT_LIST_HEAD(&(actual->childs));

	union task_union *t_actual = (union task_union*) actual;
	// ho passem a task_union perque KERNEL_ESP treballa amb task_union
	tss.esp0 = KERNEL_ESP((union task_union *) actual);	

	write_msr(0x175, (long) tss.esp0);
	set_cr3(actual->dir_pages_baseAddr);

	task1_task = actual;
}

void init_sched()
{
	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);
	INIT_LIST_HEAD(&blocked);

	for(int i = 0; i < sizeof(task)/sizeof(task[0]); ++i){
		list_add(&(task[i].task.list), &freequeue);
	}
}

int get_quantum(struct task_struct *t){
	return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum){
	t->quantum = new_quantum;
}

void update_sched_data_rr(void){
	--ticks_since_cpu;
}

// Es canvia quan el quantum arriba a 0.
int needs_sched_rr(void){
	if(ticks_since_cpu > 0) return 0;

	if(list_empty(&readyqueue)){
		ticks_since_cpu = get_quantum(current());
		return 0;
	}

	return 1;
}


void update_process_state_rr(struct task_struct *t, struct list_head *dst_queue){
	if(current()->state != ST_BLOCKED){
		if(t->state != ST_RUN) list_del(&(t->list));
		if(dst_queue != NULL){
			list_add_tail(&(t->list), dst_queue);
			if(dst_queue == &readyqueue){
				t->state = ST_READY;
			}
		} else {
			t->state = ST_RUN;
		}
	}
}

//Canviar al següent procés que treiem de la readyqueue
void sched_next_rr(void){
	struct list_head *lhead;
	struct task_struct *tstruct;

	if(!list_empty(&readyqueue)){
		lhead = list_first(&readyqueue);
		list_del(lhead);

		tstruct = list_head_to_task_struct(lhead);
	} else {
		tstruct = idle_task;
	}
	tstruct->state = ST_RUN;
	ticks_since_cpu = tstruct->quantum;
	task_switch(tstruct);
}

//Serveix per anar ciclant entre tots els processos
void schedule(void){
	update_sched_data_rr();
	if(needs_sched_rr()){
		update_process_state_rr(current(), &readyqueue);
		sched_next_rr();
	}
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

struct task_struct *list_head_to_task_struct(struct list_head *l){
	return (struct task_struct *)((long)l&0xfffff000);
}
