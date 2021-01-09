/*
  _           _   
   | . |   | |_|  Procedures for initializing IDT and handle irqs
  \  | | | | |    * 17/12/2001: start
                  * 09-jan-2005: redisign for using irq's
*/

#include <kernel.h>
#include <console.h>
#include <asm/386io.h>
#include <i386.h>


#define TRAP_HANDLER(name,ex_numb)\
		void _trap_##name(uint16_t err){\
		  def_mesg(err,ex_numb);\
		  while(1);\
		}

#define TRAP_GATE(name) (uint32_t)(void*)&_trap_##name

char* _exc_msg[]={
  "Divide by Zero",
  "Debug",
  "Non Maskable Interrupt",
  "Breakpoint",
  "Interrupt on Overflow",
  "Array Bounds Error",
  "Illegal Opcode",
  "Math not available",
  "Double Fault",
  "Math segment overflow",
  "Invalid TSS",
  "Segment not present",
  "Stack Fault",
  "General Protection Error",
  "na",
  "na",
  "Math Error",
  "Alignment Error",
  "Machine Check"
};

/* Default fault message */
void def_mesg(uint16_t err,int num){
  kprintf("KERNEL EXCEPTION> error code = 0x%x ( %s )\n",err,_exc_msg[num]);
  disable_interrupts();
}

/* Setup trap handlers */
TRAP_HANDLER(divide,	0);
TRAP_HANDLER(debug,		1);
TRAP_HANDLER(nmi,		2);
TRAP_HANDLER(breakpoint,3);
TRAP_HANDLER(overflow,	4);
TRAP_HANDLER(bounds,	5);
TRAP_HANDLER(opcode,	6);
TRAP_HANDLER(mathgone,	7);
TRAP_HANDLER(double,	8);
TRAP_HANDLER(mathover,	9);
TRAP_HANDLER(tss,		10);
TRAP_HANDLER(segment,	11);
TRAP_HANDLER(stack,		12);
TRAP_HANDLER(general,	13);
/* TRAP_HANDLER(page, 14); */
TRAP_HANDLER(matherr,	16);
TRAP_HANDLER(align,		17);
TRAP_HANDLER(mce,		18);

/**
 * IRQ stuff: macroses and externs (see isr.S for details)
 */
#define	IRQ(x) extern void irq_##x(void);
#define	SETIRQ(x,y) setup_idt_entry(&K.idt[x],KERNEL_CODE_SEG,(uint32_t)irq_##y,ACS_INT,0)

IRQ(0x01); IRQ(0x02); IRQ(0x03); IRQ(0x04);
IRQ(0x05); IRQ(0x06); IRQ(0x07); IRQ(0x08);
IRQ(0x09); IRQ(0x0a); IRQ(0x0b); IRQ(0x0c);
IRQ(0x0d); IRQ(0x0e); IRQ(0x0f);

/* Init IDT */
void init_idt(){
  int i;

 /* Disable all interrupts */ 
  disable_interrupts();

 /* disable NMIs */
#ifndef KERNEL_DEBUG 	
  outb(0x70,inb(0x70)|0x80);  /* in this place BOCHS show unsupported cmos read, addr=70******* ??? */
#endif	

 /* setup IDT table */
  setup_idt_table();

 /* setup PIC */
  setup_PIC(0x20,0x28);

 /* masks all IRQ'S */
  for(i=0;i<16;i++)
	mask_irq(i);

 /* enable timer and keyboard irqs */
  unmask_irq(0);
  unmask_irq(1);

 /* set new timer period */
  setup_PIT();

 /* allow interrupts */
  enable_interrupts();
}


/**
 * Setup entry in the idt 
 */
void setup_idt_entry(idt_t* item,uint16_t selector,uint32_t offset,
					 uint8_t access,uint8_t param_cnt){
  uint16_t _la = offset & 0xFFFF, _ha = offset >> 16;
  item->selector  = selector;
  item->offset_l  = _la; 
  item->offset_h  = _ha; 
  item->access    = access;
  item->param_cnt = param_cnt;
}


/**
 * Setup idt table:
 *  set isr's and irq's
 */
void setup_idt_table(){
    int i;
  /* set empty isr stubs */	  
    for(i=0;i<0xFF;i++)
	  setup_idt_entry(&K.idt[i],KERNEL_CODE_SEG,(uint32_t)empty_isr,ACS_INT,0);

  /* setup the IRQ's */
	SETIRQ(0x21,0x01);	SETIRQ(0x22,0x02);	SETIRQ(0x23,0x03);
	SETIRQ(0x24,0x04);	SETIRQ(0x25,0x05);	SETIRQ(0x26,0x06);
	SETIRQ(0x27,0x07);	SETIRQ(0x28,0x08);	SETIRQ(0x29,0x09);
	SETIRQ(0x2a,0x0a);	SETIRQ(0x2b,0x0b);	SETIRQ(0x2c,0x0c);
	SETIRQ(0x2d,0x0d);	SETIRQ(0x2e,0x0e);	SETIRQ(0x2f,0x0f);

/* 	setup_idt_entry(&K.idt[0], KERNEL_CODE_SEG, TRAP_GATE(divide),	  ACS_TRAP, 0); */
 	setup_idt_entry(&K.idt[1], KERNEL_CODE_SEG, TRAP_GATE(debug), 	  ACS_TRAP, 0); 
/* 	setup_idt_entry(&K.idt[2], KERNEL_CODE_SEG, TRAP_GATE(nmi), 	  ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[3], KERNEL_CODE_SEG, TRAP_GATE(breakpoint),ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[4], KERNEL_CODE_SEG, TRAP_GATE(overflow),  ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[5], KERNEL_CODE_SEG, TRAP_GATE(bounds), 	  ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[6], KERNEL_CODE_SEG, TRAP_GATE(opcode), 	  ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[7], KERNEL_CODE_SEG, TRAP_GATE(mathgone),  ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[8], KERNEL_CODE_SEG, TRAP_GATE(double), 	  ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[9], KERNEL_CODE_SEG, TRAP_GATE(mathover),  ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[10],KERNEL_CODE_SEG, TRAP_GATE(tss), 	  ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[11],KERNEL_CODE_SEG, TRAP_GATE(segment),   ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[12],KERNEL_CODE_SEG, TRAP_GATE(stack), 	  ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[13],KERNEL_CODE_SEG, TRAP_GATE(general),   ACS_TRAP, 0); */
	/* setup_idt_entry( &idt[14], KERNEL_CODE_SEG, TRAP_GATE(page), 	   ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[16],KERNEL_CODE_SEG, TRAP_GATE(matherr),   ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[17],KERNEL_CODE_SEG, TRAP_GATE(align), 	  ACS_TRAP, 0); */
/* 	setup_idt_entry(&K.idt[18],KERNEL_CODE_SEG, TRAP_GATE(mce), 	  ACS_TRAP, 0); */

  /* setup clock and kbd interrupt handlers */
    setup_idt_entry(&K.idt[0x20], KERNEL_CODE_SEG, (uint32_t)timer_irq, ACS_INT,0);
	
  /* Setup "Page fault" handler */	
    setup_idt_entry(&K.idt[0x0E], KERNEL_CODE_SEG, (uint32_t)pagefault0E, ACS_INT,0);

  /* setting up the IDTR register */
    lidt(K.idtr,K.idt);
}


/* It's not be here !!! Only for test !!! */
char *x = "-\\|/Q";

void timer_handler(){
  uchar *m = (uchar*)(VIDEO_ADDR+(NUM_COLS*K.vrow+K.vcolum)*2);
  static int xi = 0;
  if(!(K.kernel_timer%150)) {
	if(x[xi]!='Q') { *m = x[xi++]; }
	else xi = 0;
  }
  
  K.kernel_timer++;
  
 /* 15-oct-2003 */
  outb(PORT_8259M,EOI);
  scheduler();
}

void kbd_handler(){
  volatile char scancode;
 
 /* Get scan code */
  scancode = inb(PORT_KBD_A);
	
 /* Some command functionality */
  switch(scancode){
	
  case 0x32: /* m */
	show_memory_map(0,0x400000); 
	break;
	
  case 0x2E: /* c */
	init_vconsole(); 
	break;
	
  case 0x20: /* d */
	dump_page_tables(K.pg_dir);
	break;
	
  case 0x26: /* l */
	banner(); 
	break;
	
  case 0x11: /* w */
/* 	test_kmm(); */
	ktrace_stack();
	break; // why grow kernel.img ??? - ( 'coz .rodata was wrong place )
		
  case 0x10: /* q */
	sys_reset(); 
	break; 
	
  case 0x14: /* t */
	test_process_add();
	break; 

  case 0x19: /* p */
	kprint_ps();
//	__asm__ __volatile__ ("int $0x01");
	break; 
	
/*     default: kprintf( "%x\n", *h ); break; */
  }
} 


/**
 * IRQ dispath routine
 */
void dispatch_irq(int number){
  switch(number){
  case 0x01: kbd_handler(); break;
  }
}
