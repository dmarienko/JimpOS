/*
  _           _   
   | . |   | |_|  Module for working with processes
  \  | | | | |    * 13-jul-2003 Started
				  * 08-aug-2003 context_switch(...) and 4 tasks
				  * 07-dec-2004 tested spin locks functionality
				  * 10-jan-2005 added additional parameter in thread's function
*/

#include <kernel.h>
#include <console.h>
#include <kmalloc.h>
#include <i386.h>


extern void thread_bootstrap();
extern void context_switch(uint32_t* old_stack,uint32_t new_stack,uint32_t pdbr);

/**
 * This function called when thread finished
 */
void kthread_terminate(){
kprintf("\t> terminate: %d\n",K.current_thread->pid);
  K.current_thread->state = KTHREAD_DEAD;
  scheduler();
}


void idle(){
  while(1) __asm__("hlt\n");
}


/* Test spinlock */
spinlock_t testlock;

void task1(void* arg){
  int z = 3;
 spin_lock(&testlock);
  while(1){
	int i = 0;
	kprintf("TASK 1\n");
	for(i=0;i<100000LL;i++);
	if(--z<=0) break;
  }
  kprintf("TASK 1: DONE\n");
 spin_unlock(&testlock);
}

void task2(void* arg){
  int z = 3;
 spin_lock(&testlock);
  while(1){
	int i = 0;
	kprintf("TASK 2\n");
	for(i=0;i<100000LL;i++);
	if(--z<=0) break;
  }
  kprintf("TASK 2: DONE\n");
 spin_unlock(&testlock);
  kcreate_thread(task1,10);
}



void taskX1(void* arg){
  int z = 5;
  while(1){
	int i = 0;
   //kprintf("<X1> %x\n",arg);
	for(i=0; i<100000LL;i++);
	if(--z<=0) break;	
  }
}

void taskMX(void* arg){
  int z = 3;
  while(1){
	int i = 0;
   //kprintf("<MX>\n");
	for(i=0; i<100000LL;i++);
	if(--z<=0) break;
	kcreate_thread(taskX1,10,0x12345678);
  }
 //kprintf("<MX>: done\n");
  kcreate_thread(taskMX,10,NULL);
}


/**
 * Create process and return tss pointer
 *  21-dec-2004: realisation linked list for the tasks
 */
kthread_t* create_kernel_thread(uint32_t* task_code,uint16_t prio,void* arg){
  kthread_t* new_thread;
  tss_t*     task;
  uint32_t*  sp;
  int        gdt_slot;

disable_interrupts();
 
 /* Lock */
  spin_lock(&K.kcreate_thread_lock);
  
  gdt_slot = find_free_gdt_slot(TSS_GDT_START);
  if(gdt_slot==-1){
	kprintf("kcreate_thread> Can't find any free gdt slot\n");
	return NULL;
  }

 /* Allocate memory for structures */
  new_thread = (kthread_t*) kmalloc(sizeof(kthread_t));
  task = new_thread->tss = (tss_t*) kmalloc(sizeof(tss_t));
  new_thread->stack = (uint8_t*) kmalloc(THREAD_STACK_SIZE);
  
  task->trace = 0;
  task->io_map_addr = sizeof(tss_t);
  task->ldtr = 0;
  task->fs = task->gs = 0;
  task->ds = task->es = task->ss = KERNEL_DATA_SEG;
  task->cs = KERNEL_CODE_SEG;
  task->cr3 = get_CR3();  /* need to set cr3 ! */
  task->eflags = 0x202;
 /* esp must point on the top of the stack #15-oct-2003 */
  task->esp = (uint32_t) new_thread->stack + THREAD_STACK_SIZE - 1;
  task->eip = (uint32_t) task_code;

 /* fill stack */
  sp = (uint32_t*) task->esp;
  sp--; *sp = (uint32_t) arg; //KERNEL_DATA_SEG; /* Kernel data selector it would first arg in 'task_code' */
  sp--; *sp = (uint32_t) &kthread_terminate;/* Here we return after exit from thread */
  sp--; *sp = 0x202;                        /* flags - interrupts eabled - from MINIX */
  sp--; *sp = task->cs;                     /* Task code selector */
  sp--; *sp = (uint32_t) task_code;         /* here and above for ret from thread_bootstrap*/
  sp--; *sp = (uint32_t) thread_bootstrap;  /* here for ret from 'context_switch' */
  sp--; *sp = 0x3001;   /* eaf - point 5 */
  sp--; *sp = 0;        /* ebp - point 4 */
  sp--; *sp = 0;        /* esi - point 3 */
  sp--; *sp = 0;        /* edi - point 2 */
  sp--; *sp = 0;        /* ebx - point 1 */
  task->esp = (uint32_t) sp;
  
 /* Add to the gdt */	
  setup_gdt_entry(&K.gdt[gdt_slot],(uint32_t) task,sizeof(tss_t),ACS_TSS,0);
  
 /* reload GDTR with new item */
  lgdt(K.gdtr,NUM_GDT_ENTRIES,&K.gdt[0]);

 /* Initialize thread's fields */
  new_thread->pid = K.current_pid++;
  new_thread->priority = prio;
  new_thread->state = KTHREAD_READY;
  new_thread->gdt_slot = gdt_slot;
  new_thread->next = NULL;
  
  K.total_tasks++;

 /* Unlock */
  spin_unlock(&K.kcreate_thread_lock);

enable_interrupts();
 
  return new_thread;
}


/**
 * Wrapper around create_kernel_thread function
 *   create thread and add to the list
 */
kthread_t* kcreate_thread(uint32_t* task_code,uint16_t prio,void* arg){
  kthread_t* t = create_kernel_thread(task_code,prio,arg),*p,*n;
  if(t){
   /* Seeking for the last element in the list */
	for(n=p=K.global_task_table; n!=NULL;n=n->next) p = n;
	if(p){
	  p->next = t;
	  t->prev = p;
	}
  }
  return t;
}


/**
 *  Dedug ps utility :)
 *   shows all running threads and it's state
 */
void kprint_ps(){
  kthread_t* p;
  char* stats[] = {"RUNNING","READY","WAIT","STOP","DEAD"};
  kprintf("PID\tSTAT\n");
  for(p=K.global_task_table;p!=NULL;p=p->next)
	kprintf("%d\t%s\n",p->pid,stats[p->state]);
}

/**
 * Initialize sheduler
 *  it's create idle thread (pid=0)
 */
void init_scheduler(){
  kthread_t* foo_thread;
  K.scheduler_age = 0x7fffffff;
  
 /* Create empty task */
  foo_thread = create_kernel_thread(NULL,0,NULL);
  if(foo_thread==NULL)
	while(1);
  foo_thread->prev = NULL;
  K.global_task_table = foo_thread;
  
 /* Create idle thread */
  kcreate_thread((uint32_t*)&idle,0,NULL);
  K.current_thread = K.global_task_table;
}



/**
 *  test thread's function
 */
void test_process_add(){
/*   kcreate_thread((uint32_t*)&task1,10,NULL); */
/*   kcreate_thread((uint32_t*)&task2,10,NULL); */
  kcreate_thread((uint32_t*)&taskMX,10,NULL);
}


/**
 *  First scheduler - for testing
 */
void scheduler(){
  uint32_t*  paddr, pid;
  kthread_t* curr = K.current_thread;
  
disable_interrupts(); 

 /* nothing to do if no any task loaded */
  if(!curr) return;
  
  paddr = (uint32_t*) &(curr->tss->esp);
  pid = curr->pid;
  
 /* primitive round-robin - get the next task */
  if(curr->next==NULL){
	curr = K.global_task_table->next; // skip 0 task !
  } else curr = curr->next;

 /* Remove dead thread */
  if(curr->state==KTHREAD_DEAD){
	kthread_t* p,* n;
	p = curr->prev;
	n = curr->next;

	if(p!=NULL) p->next = n;
	if(n!=NULL) n->prev = p;

   /* Clear gdt slot */
	clear_gdt_entry(curr->gdt_slot);
	
   /* TODO: Free memory:  BUGS WHEN memory clearing: 24-dec-2004 !!! */
	kfree(curr->tss);
	kfree(curr->stack);
	kfree(curr);
	
	curr = NULL;
	if(n!=NULL){
	  curr = n;
	} else if(p!=NULL) curr = p;
  }

 /* switch to selected task */
  if(curr!=NULL && pid!=curr->pid){
	K.current_thread = curr;
//	curr->state = KTHREAD_RUNNING;
	context_switch(paddr,K.current_thread->tss->esp,K.current_thread->tss->cr3);
  }
  
enable_interrupts();
 
}
