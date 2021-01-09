OUTPUT_FORMAT("elf32-i386")
ENTRY(start32)

virt = 0x100000;
phys = 0x0;

SECTIONS {   
    .text virt : AT( phys ) {   
      code = .;
      *(.text)
      _etext = .;
      etext = _etext;
    }
    
    .data : AT( phys + ( data - code ) ) {   
       data = .;
       *(.data)
       *(.rodata)   /* Here we place .rodata -> ok with switch statements */
       _edata = .;
       edata = _edata;
    }

    .bss : AT( phys + ( bss - data ) ) { 
       bss = .;
       . = ALIGN( 0x1000 );       
       *(.bss)
       *(COMMON)
       _end = .; 
       end = _end;
   }
}
