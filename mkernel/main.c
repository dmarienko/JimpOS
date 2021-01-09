#include <kernel.h>
#include <console.h>

#include "build.inc"

void banner(){
  kprintf("\n*                  Jimp microkernel  / build: %s /\n",BUILD); 
  kprintf(  "*   _           _  Console commands (test mode): \n"); 
  kprintf(  "*    | . |   | |_| \t(m) - sys map\t(d) - dump table\t(w) - test kmm\n");
  kprintf(  "*   \\  | | | | |  \t(c) - clear\t(l) - logo\t(q) - quit\n\n");  
}

/* This fields fill in the head.S */
uint32_t m_start, bios_m;

/**
 * Some kernel parameters initialization 
 */
void init_kernel(){
  K.kernel_timer = 0;
  K.mem_start = m_start;
  K.bios_memory_size = bios_m;
  K.current_pid = 0;
  K.total_tasks = 0;
  K.current_thread = NULL;
}

/**
 * Main procedure - jump here from head.S 
 */
void __mkernel_main(){
  int err;

 /* Initialize kernel structure */
  init_kernel();

 /* Setting up GDT */
  setup_gdt_table();
	
 /* Init console */	
  init_vconsole();

 /* Show banner */	
  banner();
	
 /* Try to count memory ( direct probing ) */	
  K.mem_end = count_memory();
	
 /* Initialize paging mode */
  err = init_paging();
  if(err<0){
	kprintf("Init paging error: %d\n",-err);
	while(1);
  }
	
 /* Initialize interrupts descriptor table */
  init_idt();
	
 /* Initialize kernel memory manager */
  init_kmm();

 /* Initialize kernel sheduler */
  init_scheduler();

  while(1);
}

/* const char classes[] __attribute__((section(".classes"))) = "CLASSES PLACE HERE"; */
