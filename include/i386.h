#ifndef __I386_H_
#define __I386_H_

#include <defs.h>
#include <types.h>
#include <asm/386io.h>

void lgdt(gdtr_t gdtr,uint32_t num_gdt,gdt_t* gdt_offs);
void lidt(idtr_t idtr,idt_t* idt_offs);

void unmask_irq(int irq);
void mask_irq(int irq);

void setup_PIC(uint8_t v_master,uint8_t v_slave);
void setup_PIT();

void kdelay(int c);
void banner();   

void sys_reset();

#endif 
