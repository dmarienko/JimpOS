#ifndef _KERNEL_H
#define _KERNEL_H

#include <defs.h>
#include <asm/spinlock.h>

#pragma pack(1)  /* align structures to a byte boundary */

/* Segment desciptor definition */
typedef struct {
  uint16_t limit;
  uint16_t base_l;
  uint8_t  base_m;
  uint8_t  access;
  uint8_t  attribs;
  uint8_t  base_h;
} gdt_t;

/* Interrupt desciptor definition */
typedef struct {
  uint16_t offset_l;
  uint16_t selector;
  uint8_t  param_cnt;
  uint8_t  access;
  uint16_t offset_h;
} idt_t;

/* gdtr register definition */
typedef struct {
  uint16_t  limit;
  uint32_t  base;
} gdtr_t;

/* idtr register definition */
typedef struct {
  uint16_t  limit;
  uint32_t  base;
} idtr_t;

#define KTHREAD_RUNNING     0
#define KTHREAD_READY       1
#define KTHREAD_WAIT        2
#define KTHREAD_STOP        3
#define KTHREAD_DEAD        4

/* TSS definition */
typedef struct {
  uint32_t link;
  uint32_t esp0;
  uint32_t ss0;
  uint32_t esp1;
  uint32_t ss1;
  uint32_t esp2;
  uint32_t ss2;
  uint32_t cr3;
  uint32_t eip;
  uint32_t eflags;
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
  uint32_t es;
  uint32_t cs;
  uint32_t ss;
  uint32_t ds;
  uint32_t fs;
  uint32_t gs;
  uint32_t ldtr;
  uint16_t trace;
  uint16_t io_map_addr;
  uint16_t task_state;
} tss_t;


/**
 *  New kernel thread stuff
 */
typedef struct _kthread_t {
  uint16_t   pid;
  uint16_t   priority; 
  uint16_t   state;
  tss_t*     tss;
  uint32_t   gdt_slot;
  uint8_t*   stack;
  struct _kthread_t* next,* prev;
} kthread_t;


/**
 *  Main kernel structure 
 *  all kernel configuration and data here
 */
typedef struct _S_kernel {
 /* timer ticks */
  volatile uint32_t kernel_timer;

 /* Kernel GDT */
  gdt_t  gdt[NUM_GDT_ENTRIES];
  gdtr_t gdtr;
  int    gdt_slot;  /* free gdt slot */

 /* Kernel IDT */
  idt_t  idt[NUM_IDT_ENTRIES];
  idtr_t idtr;				

 /* Kernel page and tab directories */
  uint32_t* pg_tab_0;
  uint32_t* pg_dir;

  /* Memory stuff */
  uint32_t mem_start;         /* Where kernel end - heap start */
  uint32_t mem_end;           /* Total memory end - obtained by memory probing */
  uint32_t bios_memory_size;  /* Memory size obtained by bios int 0x15 function */

 /* Kernel bitmap 32 bytes per 1 MB */
  uint32_t KBITMAP_SIZE;      /* still not used */
  uint8_t  kbitmap[128];

 /* Threads */
  kthread_t* global_task_table, *current_thread;
  int        current_pid, total_tasks;
  spinlock_t kcreate_thread_lock;
  uint32_t   scheduler_age;

 /* Kernel DEBUG console */
  uint8_t    vcolum, vrow;
} kernel_t;

#pragma pack()      /* align structures to default boundary */

#include <idt.h>
#include <gdt.h>
#include <mm.h>
#include <mpaging.h>

/* Main kernel data */
kernel_t K;

#endif
