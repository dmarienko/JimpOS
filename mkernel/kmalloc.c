/*
  _           _ 
   | . |   | |_|  Kernel memory allocator 
  \  | | | | |    * 12-dec-2002  test version started 
                  * 15-dec-2002  adapting to the kernel
				  * 30-may-2003  add unmapping unused physical pages in the kfree()
				  * 01-dec-2003  rewrite kfree() function and fixed some bugs
				  
   mem_st[] :                                     [st_top-1]   [st_top]
   +------------+------------+------------+      +------------+-----
   | +block1_sz | -block2_sz | +block3_sz | .... | +blockN_sz |  NULL  ....
   +------------+------------+------------+      +------------+----- 
	
    Each stack record contain size of the memory block. If block was allocated it marked 
	by negative size value.
	
	Start address of the M'ths block defined as MEMORY_START + SUM( abs(mem_st[i]), i=0..M );
	_kfree() function may be called _gc (garbage collector) function if freed block have 
	unused prev or next blocks:

	     N           N+1          N+2
   +------------+------------+------------+ 
   | +block1_sz | -block2_sz | -block3_sz | 
   +------------+------------+------------+ 

   after calling _kfree( block2 ) it will have next view : ( block1 also unused )

                 N                 N+1
   +--------------------------+------------+ 
   | +(block1_sz + block2_sz) | -block3_sz | 
   +--------------------------+------------+ 

   Initial stack configuration ( after calling kmm_init() ):

          0        st_top
   +--------------+------+ 
   | +MEMORY_SIZE | NULL | 
   +--------------+------+ 
   
*/

#include <kernel.h>
#include <kmalloc.h>
#include <console.h>
#include <i386.h>

/**
 * Define memory stack here 
 */
uint32_t mem_st[STACK_MAX], 
  MEMORY_SIZE,				 /* Total size of the memory heap */ 
  MEMORY_START;				 /* Start memory heap address */

/* Top stack pointer */
int	st_top = 0;

/**
 * Tracing memory stack utility (for debugging - in future will be removed) 
 */
void ktrace_stack(){
  int  j;
  char *s;
  uint32_t v, S, U, UN;
	
  kprintf("\n *** Malloc stack trace utility *** \n");
  S=U=UN=0;

  for(j=0;j<st_top+1;j++){
	v = mem_st[j];
	if(mem_st[j] & NEG){
	  v &= (~NEG);
	  U += v;
	} else if(mem_st[j]>0){ 
	  UN += v;
	}
	  
	S += v;
	kprintf("  0x%X", v);
	if(!((j+1)%8)) kprintf("\n");
  }
	
  if((S==U+UN)&&(S==MEMORY_SIZE)){
	s = "OK";
  } else s = "WRONG";
	
  kprintf("\n\n\tTotal memory: 0x%x b  used: 0x%x b free: 0x%x b (%s)\n",S,U,UN,s);
}

/**
 * Shrink stack starting from ptr by 1 slot
 */
int st_shrink(int ptr){
  if(st_top+1>=STACK_MAX) return -1;
  kmemmove(&mem_st[ptr+1],&mem_st[ptr],4*(st_top-ptr));
  return ++st_top;
}

/**
 * Collapse stack starting from ptr by 1 slot  
 */
int st_collapse(int ptr){
  if(st_top==1) return -1;
  kmemmove(&mem_st[ptr],&mem_st[ptr+1],4*(st_top-ptr));
  mem_st[st_top] = 0;
  return --st_top;
}

/**
 * Memory allocator init function 
 */
void kma_init(uint32_t m_start,uint32_t m_size){
  st_top = 0;
  MEMORY_START = m_start; 
  MEMORY_SIZE  = m_size; 
  mem_st[st_top++] = MEMORY_SIZE;
}

/**
 * Memory garbage collector function 
 */
void _kgc(int ptr){
  uint32_t  l1 = mem_st[ptr], l2 = mem_st[ptr+1];
  if(st_collapse(ptr)>0) mem_st[ptr] = l1 + l2;
}

/**
 * Memory allocator (kernel mode)
 */
void* kmalloc(uint32_t len) {
  uint32_t addr = MEMORY_START, x = 0, i;
	
 /* We don't allocate zero size block */	
  if(!len) return (void*)(0);
	
 /* Round len on paragraph length ( 16 bytes ) */	
  len = ALIGN(len,0x10);
	
  for(i=0;i<st_top;i++){
   /* If it's used than go to the next block */
	if(mem_st[i] & NEG) {
	  addr += (mem_st[i] ^ NEG);
	  continue;
	} 
	else {  /* else check size */
	  if(len>mem_st[i]){  /* chank is less than we need - next */
		addr += mem_st[i];
		continue;
	  }
	  else if(len==mem_st[i]){ /* OK - we find explicit equal block */
	   /* Mark block as used */	
		mem_st[i] |= NEG;
		x = addr;
		break;
	  } 
	  else {  /* len < mem_st[ i ] */
	   /* If rest of the finded block have size less than 25%  - we'll use all block space */	
		if((100*(mem_st[i]-len)/mem_st[i])<25){
		  mem_st[i] |= NEG;
		}
		else {
		 /* Try to shrink stack */
		  if(!st_shrink(i)) break;  /* cant shrink stack - exit ( bad case ) */
		  mem_st[i+1] = mem_st[i] - len;
		  mem_st[i] = (len | NEG);
		}
		x = addr;
		break;
	  }
	}
  }
  return (void*)(x);
}

/**
 * Memory deallocator (kernel mode)
 */
void kfree(void* memptr){
  uint32_t  
	addr = MEMORY_START, own_len = 0,
	ap   = MEMORY_START, lp = 0,
	an   = 0, ln = 0,
	cs,  ce;
  int i, find_ok = 0;

  for(i=0;i<st_top;i++){
   /* Check is it used block */
	if(mem_st[i]&NEG){   
	  if(addr==(uint32_t)memptr){  /* Is it need address ? */
		mem_st[i] ^= NEG; /* mark block as free */
		own_len = mem_st[i];
	   /* We are need gc ? */ 
		if(i && i<=st_top){ 
		  if((mem_st[i-1] & NEG)==0){ _kgc(i-1); i--; }  /* prev block is free */
		  if((mem_st[i+1] & NEG)==0){ _kgc(i); }		    /* next block is free */
		}
		find_ok = 1;
		break;		/* exit here */
	  }
	 /* Store previous address and length */	
	  ap = addr; lp = mem_st[i] & (~NEG); 
	}
	addr += (mem_st[i] & (~NEG));  /* Count address here */
  }

 /* If such address wasn't find - exit */	
  if(!find_ok) return;
  
 /* Try to find next used block */
  i++;
  addr = (uint32_t) memptr + own_len; 
  for(;i<st_top;i++){
	if(mem_st[i] & NEG) { an = addr; ln = mem_st[i] & (~NEG); break; }
	addr += mem_st[i]; 
  }

  if(!an) an = ((uint32_t) memptr + own_len + 0x1000);
  if(ap==MEMORY_START) ap -= 0x1000;

  cs = STRIP_PAGE(memptr);
  if(P_EQU(ap+lp,memptr)) cs += 0x1000;

  ce = STRIP_PAGE(an);
  if(P_EQU(memptr+own_len,an)) ce -= 0x1000;

 /* no unmapping needs */
  if(cs>ce) return;
  
  for(addr=cs;addr<ce;addr+=0x1000){
   /* Unmap virtual page address */
	uint32_t physic_unmapped = unmap_page(K.pg_dir,addr);

   /* Mark this physical address as unused in the bitmap */
	BITMAP_CLR(K.kbitmap,physic_unmapped);
  }

 /* We need notify that paging directory was changed ! */	
  set_CR3(get_CR3());
  memptr = 0;
}

/**
 * Check is it address memptr is valid ? (It was allocated by kmalloc ?)
 */
int kvalid_addr(uint32_t memptr){
  uint32_t  addr = MEMORY_START;
  int i;
	
 /* Check outboundings */	
  if((memptr<MEMORY_START)||(memptr>MEMORY_START+MEMORY_SIZE)) return 0;
	
  for(i=0; i<st_top; i++) {
   /* Check is it used block */
	if(mem_st[i] & NEG) {   
	  if((memptr>=addr) && (memptr<addr+(mem_st[i]&(~NEG)))) return 1;
	}
	addr+=(mem_st[i] & (~NEG));  /* Count address here */
	if(memptr < addr) break;
  }
  return 0;
}

