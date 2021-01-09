#ifndef __GDT_H
#define __GDT_H

void setup_gdt_entry(gdt_t* item,uint32_t base,uint32_t limit,uchar access,uchar attribs);
void setup_gdt_table();
void clear_gdt_entry(int slot);
int  find_free_gdt_slot(int start_slot);
	 
#endif
