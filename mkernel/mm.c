/*
  _           _ 
   | . |   | |_|  Memory manager
  \  | | | | |    * 26-nov-2002  started 
                  * 16-dec-2002  modifyed for kmalloc ( added pagefault_handler )   
				  * 02-mar-2004  skip upper memory blocks in scan_memory_map()
*/

#include <kernel.h>
#include <kmalloc.h>
#include <console.h>
#include <i386.h>

/**
 * Setting memory area to val 
 */
/* void kmemset(void *dst,uchar val,uint32_t len){ */
/*   __asm__ __volatile__ (	 */
/* 						"rep stosb\n" */
/* 						:: "D"(dst), "a"(val), "c"(len)  */
/* 						); */
/* } */


/** 
 * Copying len memory area from src to dst 
 *  very dumb function !!! ( was )
 *  now ( 15-dec-2002 ) it's rewriten on assembler :) 
 */
/* void kmemcpy(void *dst,void *src,uint32_t len){ */
/*   __asm__ __volatile__ (	 */
/* 						"cld\n" */
/* 						"rep movsb\n" */
/* 						:: "D"(dst), "S"(src), "c"(len)  */
/* 						); */
/* } */

/**
 * Moving len memory area from src to dst 
 */
/* void *kmemmove(void *dest,const void *src,uint32_t len) { */
/*   char		 *d=dest; */
/*   const char *s=src; */

/*   if(s<d && (s+len)>d) { */
/*    /\* pointers overlapping, have to copy backwards *\/ */
/* 	for(;;) { */
/* 	  --len; */
/* 	  if(len==(uint32_t)-1) break; */
/* 	  d[len] = s[len]; */
/* 	} */
/*   } else { */
/*    /\* copy forwards *\/ */
/* 	while(len!=0) { */
/* 	  *d++ = *s++; */
/* 	  --len; */
/* 	} */
/*   } */
/*   return dest; */
/* } */

/**
 * Set memory in the kbitmap as used 
 */
void set_memory_map(uint32_t mstart,uint32_t mend){
  uint32_t a;
  for(a=mstart;a<mend;a+=0x1000) BITMAP_SET(K.kbitmap, a);
}

/**
 * Show system memory map (physical) 
 */
void show_memory_map(uint32_t mstart, uint32_t mend){
  uint32_t  a, i=1;
  
  kprintf(" *** Physical memory map usage: from 0x%x to 0x%x ***\n\t",mstart,mend);
  for(a=mstart;a<mend;a+=0x1000){
	if(BITMAP_TEST(K.kbitmap,a)){ kprintf("\x8");}
	else kprintf(".");
	if(!(i++ % 64)) kprintf("\n\t"); 
  }
  kprintf("\n");
}

/**
 * Scanning memory map and get free page
 */
uint32_t scan_memory_map(){
  uint32_t i;
  uint8_t  j, mask;

 /* Skip memory untill video and video segment */	
 /* And skip upper memory blocks fixed: /02-mar-2004/ */
/*   for(i=B_IDX(VIDEO_ADDR);i<sizeof(K.kbitmap);i++){ */
  for(i=B_IDX(UMA_END);i<sizeof(K.kbitmap);i++){
	if(K.kbitmap[i]==0xff) continue;
	mask=0x80;
	for(j=0;j<8;j++,mask>>=1){
	  if(!(K.kbitmap[i]&mask)) return (uint32_t)((i*8+j)*4096);
	}
  }
  kprintf("PANIC> Cannot find any free page\n");
  return 0;
}

/**
 * Init memory manager 
 */
void init_kmm() {
  K.KBITMAP_SIZE = K.mem_end>>15; /* = PhMemory/8*0x1000 -> >>15 */
  kma_init(ROUND_PAGE(K.mem_start),0x400000-ROUND_PAGE(K.mem_start));

#ifdef	KERNEL_DEBUG
  kprintf( "kmm> heap memory start at = 0x%X\n", K.mem_start );
  kprintf( "kmm> memory end at = 0x%X\n" , K.mem_end );
  kprintf( "kmm> bios_memory_size value = %d \n", K.bios_memory_size + 1024 ); 
  kprintf( "kmm> for bitmap need = %d bytes (%d pages)\n", K.KBITMAP_SIZE, K.KBITMAP_SIZE>>12);
#endif	
}

/**
 *  Page fault memory handler 
 *  used by 1'st memory allocator
 */
void pagefault_handler(uint32_t err){
  uint32_t addr;
	
 /* Get fault address */
  addr = get_CR2();
  err &= 0x7;

 /* Check is it valid address ? */	
  if(!kvalid_addr(addr)){ 
	kprintf("PAGE FAULT(err=0x%x)> memory at 0x%x was not allocated\n",err,addr);
	for(;;);
  } else {
	uint32_t mp, mv;
	mp = scan_memory_map();
	mv = STRIP_PAGE(addr); 
   
#ifdef	KERNEL_DEBUG
   kprintf("PAGE FAULT(err = 0x%x)> map virtual address = 0x%x <- 0x%x\n",err,mv,mp);
#endif
	
	if(map_mem(K.pg_dir,PRIV_WRITABLE|PRIV_PRESENT,mv,mp,0x1000)!=0){
	  kprintf("PANIC> Cannot map (virt) 0x%x <- (phys) 0x%x\n",mv,mp);
	  for(;;);
	}
	
   /* Set _physical_ page as used */
	BITMAP_SET(K.kbitmap,mp);
	
   /* Clear mapped page ??? */
	kmemset((void*)(mv),0,0x1000-1);
  }
}

/**
 * Test kernel memory manager 
 * debug
 */
void test_kmm() {
	char *a;
	int i;
/* 		ktrace_stack(); */
	a = (char*) kmalloc( 5 * 0x1003 );
/* 		a = (char*) kmalloc( 0xe00 ); */
/* 		ktrace_stack(); */
	
  /* Test page fault handler */
	kmemset(a,0x1,5*0x1003-1);
/*	a[2*0x1005] = '1';*/
	kfree(a);
/* 	ktrace_stack(); */
}

