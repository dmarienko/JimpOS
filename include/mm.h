#ifndef __MM_H
#define __MM_H

#include <mpaging.h>

/* BUG: was w/o (...) */
#define  ROUND_PAGE(x)		(((uint32_t) x + 4096 - 1 ) & 0xFFFFF000L)    
#define  STRIP_PAGE(x)		(((uint32_t) x ) & 0xFFFFF000L)    

/* Bitmap's macroses */
#define	 B_IDX(x)			(x>>12)/8
#define	 B_BIT(x)			(uint8_t)(0x80 >>(( x>>12)%8))
#define	 BITMAP_TEST(b,x)	(b[B_IDX(x)] & B_BIT(x)) ? 1 : 0
#define	 BITMAP_SET(b,x)	b[B_IDX(x)] |=  B_BIT(x)
#define	 BITMAP_CLR(b,x)	b[B_IDX(x)] &=~ B_BIT(x)

/* Memory manipulation functions */
void	kmemset( void *dst, uchar val, uint32_t len );
void	kmemcpy( void *dst, void *src, uint32_t len );
void	*kmemmove( void *dest, const void *src, uint32_t len );

/* Show system memory */
void	 show_memory_map( uint32_t mstart, uint32_t mend );
uint32_t scan_memory_map();
void	 set_memory_map( uint32_t mstart, uint32_t mend );

/* Kernel memory manager */
void	init_kmm();

#endif
