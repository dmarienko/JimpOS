/*
  _           _  
   | . |   | |_|  IDT for x86 platform
  \  | | | | |    

*/

#include <kernel.h>
#include <i386.h>

/**
 * Loading GDT register and apply it 
 */
void lgdt(gdtr_t gdtr,uint32_t num_gdt,gdt_t* gdt_offs){
 /* Setup gdtr */
  gdtr.limit = (num_gdt*sizeof(gdt_t))-1;
  gdtr.base = (uint32_t) gdt_offs;
	
 /* Load gdtr register */
  lgdtr((uint32_t)&gdtr);
  __asm__ __volatile__("movl $0x10, %%eax\n"
					   "movw %%ax, %%ds\n"
					   "movw %%ax, %%es\n"
					   "movw %%ax, %%fs\n"
					   "movw %%ax, %%gs\n"
					   "movw %%ax, %%ss":::"eax","cc");

 /* CS has old selector 0x18 and when far call occured
	stack has return addres = (0x18,addr) */
  __asm__ __volatile__("ljmp $0x08,$new_sel; new_sel:"::);

}

/**
 * Loading IDT register and apply it 
 */
void lidt(idtr_t idtr,idt_t* idt_offs){
  uint32_t l = (uint32_t) &idtr;

 /* Set limit and base address of IDT table */
  idtr.limit = NUM_IDT_ENTRIES*sizeof(idt_t)-1;
  idtr.base = (uint32_t) idt_offs; 
	
 /* Load idt register */
  __asm__ __volatile__("lidt (%0)" :: "r"( l ));
}

void unmask_irq(int irq){
  if(irq<8) outb(PORT_8259M+1,(inb(PORT_8259M+1) & ~(1 << irq)));
  else outb(PORT_8259S+1,(inb(PORT_8259S+1) & ~(1 << (irq-8))));
}

void mask_irq(int irq){
  if(irq<8) outb(PORT_8259M+1,(inb(PORT_8259M+1) | (1 << irq)));
  else outb(PORT_8259S+1,(inb(PORT_8259S+1) | (1 << (irq-8))));
}

void setup_PIC(uint8_t v_master,uint8_t v_slave){
  uchar a1, a2;

  a1 = inb(PORT_8259M+1);
  a2 = inb(PORT_8259S+1);

  outb(PORT_8259M,0x11);             /* start 8259 initialization */
  outb(PORT_8259S,0x11);
  outb(PORT_8259M+1,v_master);       /* master base interrupt vector */
  outb(PORT_8259S+1,v_slave);        /* slave base interrupt vector */
  outb(PORT_8259M+1,1<<2);           /* bitmask for cascade on IRQ2 */
  outb(PORT_8259S+1,2);              /* cascade on IRQ2 */
  outb(PORT_8259M+1,1);              /* finish 8259 initialization */
  outb(PORT_8259S+1,1);

  outb(PORT_8259M+1,a1);
  outb(PORT_8259S+1,a2);
}

void setup_PIT(){
 /* Set timer period = 3.004 ms  = ( 0x0e00 * .8380965 us ) */
  outb(0x43,0x24);
  outb(0x40,0x0e);
}

/**
 * System reset 
 */
void sys_reset(){
  while(inb(0x64)&0x02);
  outb(0x64,0xfe);
  __asm__ __volatile__("cli\n"
					   "hlt"::);
}
