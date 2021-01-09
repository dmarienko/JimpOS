#ifndef _DEFS_H
#define _DEFS_H

#include <types.h>

#define	PHYS_ADDR       0          /* was 0x10000L */
#define	VIDEO_ADDR      0xb8000L
#define	UMA_START       0xc0000L
#define	UMA_END         0x100000L

/* Main segments */
#define KERNEL_CODE_SEG 0x08
#define KERNEL_DATA_SEG 0x10

/* Define thread's size here */
#define THREAD_STACK_SIZE     0x1000

/* Number of IDT entries */
#define  NUM_IDT_ENTRIES	0xFF

/* Number of GDT entries */
#define  NUM_GDT_ENTRIES	0xFF
#define  TSS_GDT_START      3

/* Number rows and columns for debug console */
#define  NUM_ROWS       25
#define  NUM_COLS       80

/* Ports */
#define PORT_8259M      0x20
#define PORT_8259S      0xA0
#define PORT_KBD_A      0x60
#define EOI             0x20

/* Access byte's flags */
#define ACS_PRESENT     0x80            /* present segment */
#define ACS_CSEG        0x18            /* code segment */
#define ACS_DSEG        0x10            /* data segment */
#define ACS_CONFORM     0x04            /* conforming segment */
#define ACS_READ        0x02            /* readable segment */
#define ACS_WRITE       0x02            /* writable segment */
#define ACS_IDT         ACS_DSEG        /* segment type is the same type */
#define ACS_INT_GATE    0x0E            /* int gate */
#define ACS_INT_TRAP    0x0F            /* int trap */
#define ACS_INT         (ACS_PRESENT | ACS_INT_GATE) /* present int gate */
#define ACS_TRAP        (ACS_PRESENT | ACS_INT_TRAP) /* present int trap */
#define ACS_TSS_GATE    0x09
#define ACS_TSS         (ACS_PRESENT | ACS_TSS_GATE) /* present tss gate */


/* Ready-made values */
#define ACS_CODE        (ACS_PRESENT | ACS_CSEG | ACS_READ)
#define ACS_DATA        (ACS_PRESENT | ACS_DSEG | ACS_WRITE)
#define ACS_STACK       (ACS_PRESENT | ACS_DSEG | ACS_WRITE)

#endif

