/*
  _           _         
   | . |   | |_|  Simple console driver needed for debugging     
  \  | | | | |    * 28/09/2001    
                  * 27-dec-2003 fixed some things
*/

#include <kernel.h>
#include <console.h>
#include <mm.h>
#include <asm/386io.h>


/* Clearing kernel console */
void clear_vconsole() {
  uint16_t *m = (uint16_t*) VIDEO_ADDR;	 
  int i;
		
 /* fill spaces and white on black color mode */
  for(i=0;i<NUM_COLS*NUM_ROWS;i++) *m++ = 0x0720; 
}

/* Initialization kernel console */
void init_vconsole() {
 /* Clear and init */ 
  clear_vconsole();
  update_cursor(0,0);
  K.vcolum = K.vrow = 0;
}

/* Update cursor on the display */
void update_cursor(int row,int col) {
  uint16_t position = (row*NUM_COLS)+col;

 /* cursor LOW port to vga INDEX register */
  outb(0x3D4,0x0F);
  outb(0x3D5,(uint16_t)(position&0xFF));

 /* cursor HIGH port to vga INDEX register */
  outb(0x3D4,0x0E);
  outb(0x3D5,(uint16_t)((position>>8)&0xFF));
	
 /* if need to hide cursor */
  outb(0x3D4,0x0A); outb(0x3D5,32);
  outb(0x3D4,0x0B); outb(0x3D5,32);
}

/* CR function: fixed 04-nov-2002 */
void vcrlf() {
  uchar  *ms = (uchar*)(VIDEO_ADDR+NUM_COLS*2);
  uint16_t *ml = (uint16_t*)(VIDEO_ADDR+(NUM_COLS*(NUM_ROWS-1))*2);
  int    i;
	  
  K.vcolum = 0;
  if(K.vrow++>=NUM_ROWS-1) { 
	K.vrow--;
	
   /* Page up */ 
	kmemcpy((void*)VIDEO_ADDR,ms,2*(NUM_ROWS-1)*NUM_COLS);

   /* Clear last row */ 
	for(i=0;i<NUM_COLS;i++) *(ml+i)=0x0720;
  }
  update_cursor(K.vrow,K.vcolum);
}

/* put character on the terminal and calc new cursor position */
void kputchar(char ch) {
  uchar *m = (uchar*)(VIDEO_ADDR+(NUM_COLS*K.vrow+K.vcolum)*2);
  
  switch(ch){
  case 0x09:  /* TAB */ 
	K.vcolum = (K.vcolum+8) & ~(8-1);
	break;  
	
  case '\r':  /* Carriage return */ 
	K.vcolum = 0;
	break;  
	
  case '\n':  /* Line feed */ 
	vcrlf();
	return;
	
  default:	  /* Printable char */
	*m = ch; 
	K.vcolum++;
	break;
  }
	
  if(K.vcolum>=NUM_COLS-1) vcrlf();
  update_cursor(K.vrow,K.vcolum);
}

