#ifndef __KMALLOC_H
#define __KMALLOC_H

#define	STACK_MAX       1024				/* Maximal number of the stack's slots */
#define	NEG             0x8000000L			/* Significant bit ( for mark used slots ) */
#define	ALIGN( x, a )   (x+(a-1)) & ~(a-1)	/* Round x on bound of a at up side */
#define P_EQU(x,y)      (STRIP_PAGE(x)==STRIP_PAGE(y))

void ktrace_stack();
void kma_init(uint32_t m_start,uint32_t m_size);

int  st_shrink(int ptr);
int  st_collapse(int ptr);

void _kgc(int ptr);
void *kmalloc(uint32_t len);
void kfree(void* memptr);
int	 kvalid_addr(uint32_t memptr);

#endif
