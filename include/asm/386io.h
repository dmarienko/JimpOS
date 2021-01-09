#ifndef _386_IO_
#define _386_IO_

#define io_wait()\
	 __asm__ __volatile__ (	"jmp 1f\n"\
						"1:\tjmp 1f\n"\
						"1:" ::)

#define inb(port) ({ \
	unsigned char _v; \
	__asm__ __volatile__ ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
	_v; \
})

#define outb(port, value) \
	__asm__ __volatile__ ("outb %%al, %%dx" :: "a" (value), "d" (port))


#define inw(port) ({ \
	unsigned short _v; \
	__asm__ __volatile__ ("inw %%dx,%%ax":"=a" (_v):"d" (port)); \
	_v; \
})

#define outw(port, value) \
	__asm__ __volatile__ ("outw %%ax,%%dx"::"a" (value),"d" (port))


#define inl(port) ({ \
	unsigned long _v; \
	__asm__ __volatile__ ("inl %%dx,%%eax":"=a" (_v):"d" (port)); \
	_v; \
})

#define outl(port, value) \
	__asm__ __volatile__ ("outl %%eax,%%dx"::"a" (value),"d" (port))

#define repinsw(port, buffer, count) \
	__asm__ __volatile__ ("rep\n\t" \
					"insw" :: "d" (port), "D" (buffer), "c" (count):"edi", "edx", "ecx", "eax");

#define get_CR0()({\
	unsigned long _v;\
	__asm__ __volatile__ ("movl %%cr0, %%eax":"=a"(_v));\
	_v; \
})

#define set_CR0(c)\
	__asm__ __volatile__ ("movl %%eax, %%cr0"::"a" (c))

#define get_CR2()({\
	unsigned long _v;\
	__asm__ __volatile__ ("movl %%cr2, %%eax":"=a"(_v));\
	_v; \
})

#define get_CR3()({\
	unsigned long _v;\
	__asm__ __volatile__ ("movl %%cr3, %%eax":"=a"(_v));\
	_v; \
})

#define set_CR3(c)\
	__asm__ __volatile__ ("movl %%eax, %%cr3"::"a" (c))


#define long_jump(of,seg) {\
  struct { uint32_t offset; uint16_t segment; } _tmp; \
  __asm__( "movl %%edx, %0\n" \
  	       "movw %%cx,  %1\n" \
		   "ljmp *%0\n" \
		    :: "m" (*& _tmp.offset), "m" (*& _tmp.segment), "d" (of), "c" (seg) ); \
}

#define jmp_task(t) \
	long_jump( 0, t );

#define ltr(t)\
	__asm__ __volatile__( "ltr  %%ax":: "a" (t) );

#define lgdtr(t)\
	__asm__ __volatile__("lgdt (%0)" :: "r"( t ) );

#define disable_interrupts()({ __asm__ __volatile__ ("cli"::);})
#define enable_interrupts()({ __asm__ __volatile__ ("sti"::);})

#endif
