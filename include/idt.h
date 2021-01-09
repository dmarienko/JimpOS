#ifndef _IDT_H
#define _IDT_H

#ifdef __cplusplus
extern "C"{
#endif

extern	void  empty_isr();
extern	void  timer_irq();
extern	void  pagefault0E();

void init_idt();
void setup_idt_entry(idt_t* item,uint16_t selector,uint32_t offset,uint8_t access,uint8_t param_cnt);
void setup_idt_table();

#ifdef __cplusplus
};
#endif

#endif
