/*
  _           _ 
   | . |   | |_|  Memory paging functions
  \  | | | | |    * 11/12/2001 - initial release
                  * 24-nov-2003 - dynamic allocating kernel page/tab dirs
*/

#include <kernel.h>
#include <console.h>
#include <asm/386io.h>
#include <i386.h>

/** 
 * Setting up paging mode 
 * call from __mkernel_main
 */
int init_paging(){
  uint32_t tab;

 /* Allocate memory for page directory and clear it: 24-nov-2003 */
  K.pg_dir = (uint32_t*) ROUND_PAGE(K.mem_start);
  kmemset(K.pg_dir,0,0x1000);	

 /* Increase memory start by kernel page dir size */
  K.mem_start+=0x1000;
	
 /* Setup kernel bitmap - clear it */
  kmemset(K.kbitmap,0,128);

 /* Allocate memory for tab directory: 24-nov-2003 */
  K.pg_tab_0 = (uint32_t*) ROUND_PAGE(K.mem_start);
  kmemset(K.pg_tab_0,0,0x1000);	/* and need to free it: 11-dec-2003 */
  tab = (uint32_t) K.pg_tab_0;

 /* Increase memory start by kernel tab dir size */
  K.mem_start+=0x1000;

 /* install page dir */
  if(install_page_table(K.pg_dir,tab,0)!= 0) return -1;

 /* mapping first 4MB - for kernel 
 	if( map_mem( pg_dir, PRIV_WRITABLE | PRIV_PRESENT, 
	0, 0, 0x400000L ) != 0 ) return -1; */
  
 /* mapping video segment */
  if(map_mem(K.pg_dir,PRIV_WRITABLE|PRIV_PRESENT,
			 VIDEO_ADDR,VIDEO_ADDR,0x1000)!=0) return -2;
  
 /* Mark video segment as used in the kbitmap  */	
  set_memory_map(VIDEO_ADDR,VIDEO_ADDR+0x1000);

 /* mapping kernel code area - after video segment */
  K.mem_start += 0x1000;
  if(map_mem(K.pg_dir,PRIV_WRITABLE|PRIV_PRESENT, 
			 0x100000L,0x100000L,K.mem_start-0x100000L)!= 0) return -3;
	
 /* Mark kernel are as used in the kbitmap  
  * here we don't mark video segment as used ! */	
  set_memory_map(0x100000L, K.mem_start);
  
  /* turn on paging mode */
  setup_paging_mode((uint32_t)(K.pg_dir));

  return 0;
}

/**
 * Turn on paging mode 
 */
void setup_paging_mode(uint32_t __pg_dir){
  __asm__ __volatile__("mov %%eax, %%cr3\n"
					   "mov %%cr0, %%eax\n"
					   "orl $0x80000000, %%eax\n"
					   "mov %%eax, %%cr0"
					   :: "a"(__pg_dir)
					   );
}

/** 
 * Install page directory  
 */
static int install_page_table(uint32_t *dir, uint32_t table, uint32_t virt) {
  uint16_t dirent;

 /* put PHYSICAL address of page table into page dir */
  dirent = PDI(virt);
  if(dir[dirent] != 0) return -1;
  dir[dirent] = (table & 0xFFFFF000L)|(PRIV_USER|PRIV_WRITABLE|PRIV_PRESENT);
  return 0;
}

/**
 * Map 1 physical page (phys) -> (virt) with (priv) into (dir) 
 */
static int map_page(uint32_t *dir,uint16_t priv,uint32_t virt,uint32_t phys){
  uint16_t dirent, tabent;
  uint32_t *tab;

 /* got a page table ? */
  dirent = PDI(virt);
  if(dir[dirent] == 0) return -1;

 /* point to PHYSICAL address of page table */
  tab = (uint32_t*)( dir[dirent] & 0xFFFFF000L );

 /* map the page */
  tabent = PTI(virt);
  priv &= 0xFFF;
  tab[tabent] = (phys & 0xFFFFF000L) | priv;
  
  return 0;
}

/**
 * Unmap virt address and return mapped physical
 */
uint32_t unmap_page(uint32_t *dir,uint32_t virt){
  uint16_t dirent, tabent;
  uint32_t *tab, phys;

 /* got a page table ? */
  dirent = PDI(virt);
  if(dir[dirent] == 0) return -1; 

 /* point to PHYSICAL address of page table */
  tab = (uint32_t*)(dir[dirent] & 0xFFFFF000L);

 /* unmap the page */
  tabent = PTI(virt);
  phys = tab[tabent] & 0xFFFFF000L;
  tab[tabent] = 0;
  
  return phys;
}

/**
 * Mapping physical memory from <phys> to virtual <virt> 
 * <len> - physical memory length
 * <dir> - pointer to page directory
 */
int map_mem(uint32_t *dir,uint16_t priv,uint32_t virt,uint32_t phys,uint32_t len){
  uint32_t tvirt;

  for(tvirt = virt; tvirt < virt + len; tvirt += 0x1000) {
	if(map_page(dir, priv, tvirt, phys) != 0) return -1;
	
   /* Mark as used */ 
   //BITMAP_SET( phys );
	phys += 4096;
  }
  return 0;
}

/**
 * Memory count function - return how much memory in MB
 * not consider memory hole at 15-16 MB !
 */
uint32_t count_memory(void) {
  register uint32_t *mem;
  uint32_t mem_count, a, bse_end, mem_end, CR0;
  uint16_t memkb;
  uint8_t	 irq1, irq2;

 /* save IRQ's */
  irq1 = inb(0x21);
  irq2 = inb(0xA1);

 /* kill all irq's */
  outb(0x21,0xFF);
  outb(0xA1,0xFF);

  mem_count = 0;
  memkb = 0;

 /* store a copy of CR0 */
  CR0 = get_CR0();

 /* invalidate the cache
  * write-back and invalidate the cache */
  __asm__ __volatile__ ("wbinvd");

 /* plug cr0 with just PE/CD/NW
  * cache disable(486+), no-writeback(486+), 32bit mode(386+) */
  set_CR0(CR0|0x00000001|0x40000000|0x20000000);

  do {
	memkb++;
	mem_count+=1024*1024;
	mem=(uint32_t*)mem_count;
	a=*mem;
	*mem=0x55AA55AA;
	
   /* the empty asm calls tell gcc not to rely on whats in its registers
	* as saved variables (this gets us around GCC optimisations) */
	asm("":::"memory");
	if(*mem!=0x55AA55AA){ if(mem_count!=16) mem_count=0; }
	else {
	  *mem=0xAA55AA55;
	  asm("":::"memory");
	  if(*mem!=0xAA55AA55) mem_count=0;
	}
	
	asm("":::"memory");
	*mem=a;
  } while( memkb<4096 && mem_count!=0 );

 /* restore CR0 */
  set_CR0(CR0);

  mem_end = memkb << 20;
  mem = (uint32_t*) 0x413;
  bse_end = ((*mem)&0xFFFF)<<6;

  outb(0x21,irq1);
  outb(0xA1,irq2);
  
#ifdef	KERNEL_DEBUG
  kprintf("mpg> memory count = %d MB\n",  mem_end/(1024*1024) ); 
#endif

  return mem_end; 
}

/**
 * Function for dump contents of page_dir table
 * debug
 */
void dump_page_tables(uint32_t *page_dir) {
  uint32_t temp, *page_tab;
  uint32_t pde, pte;

  kprintf("page directory at 0x%x\n", page_dir );
  
 /* RAM */
  for( pde = 0; pde < 512; pde++ ) {
	if( page_dir[pde] == 0 ) continue;

	temp = page_dir[pde];
	kprintf("page dir[%d]=0x%x\n", pde, temp);
	page_tab = (uint32_t *)(temp & -4096);

	for( pte = 0; pte < 1024; pte++ ) {
	  if(page_tab[pte] == 0) continue;
	  temp = page_tab[pte];
	  kprintf( "\tpage tab[%d]=0x%x\n", pte, temp );
	}
  }
}

