/*
  _           _  
   | . |   | |_|   Procedures for setting up GDT  
  \  | | | | |     * 16/12/2001 
   
*/


#include <kernel.h>
#include <console.h>
#include <i386.h>

/**
 * Initialisation GDT entry  
 */
void setup_gdt_entry(gdt_t *item,uint32_t base,uint32_t limit,uchar access,uchar attribs){
  item->base_l = base & 0xFFFF;
  item->base_m = (base >> 16) & 0xFF;
  item->base_h = base >> 24;
  item->limit  = limit & 0xFFFF;
  item->attribs = attribs | ((limit >> 16) & 0x0F);
  item->access = access;
}

/**
 * Initial setting up GDT table 
 */
void setup_gdt_table(){
 /* Clear number of the current free gdt slot */
  K.gdt_slot = 0;

 /* Clear GDT initially */
  kmemset(&K.gdt[0],0,NUM_GDT_ENTRIES*sizeof(gdt_t));

 /* NULL descriptor [0x00] */
  setup_gdt_entry(&K.gdt[K.gdt_slot++],0,0,0,0);

 /* Code segment    [0x08] */
  setup_gdt_entry(&K.gdt[K.gdt_slot++],0,0xFFFFFFFFL,ACS_CODE,0xCF);

 /* Data segment    [0x10] */
  setup_gdt_entry(&K.gdt[K.gdt_slot++],0,0xFFFFFFFFL,ACS_DATA,0xCF);

 /* set GDTR */
  lgdt(K.gdtr,K.gdt_slot,&K.gdt[0]);
}


/**
 * Try to find free slot in the GDT table
 * return - slot number
 */
int find_free_gdt_slot(int start_slot){
  int i;
  for(i=start_slot;i<NUM_IDT_ENTRIES;i++){
	if(K.gdt[i].access==0)return i;
  }
  return -1;
}

/**
 * Clear gdt slot
 */
void clear_gdt_entry(int slot){
  setup_gdt_entry(&K.gdt[slot],0,0,0,0);
 //lgdt(K.gdtr,NUM_GDT_ENTRIES,&K.gdt[0]);
}
