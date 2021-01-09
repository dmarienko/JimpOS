#
#  _           _   Makefile for kernel image
#   | . |   | |_|  08/09/2001
#  \  | | | | |  
#


INCLUDE_PATH=../include
DATE=`date +%D\ %T`

# gcc compiler
CC=gcc
#CC=gcc-3.0
CC_OPTIONS= $(DEBUG_FLAGS) -O2 -c -s -I$(INCLUDE_PATH) -nostdlib -nostdinc++ \
			-nostdinc -fno-builtin -fomit-frame-pointer -Wimplicit \
			-fwritable-strings -ffreestanding

LIBC_PATH=../libc
KERNEL_LIBC=$(LIBC_PATH)/libc.a

# linker
LINK=ld
LINK_OPTIONS=-Map kernel.map -s --no-check-sections --gc-sections --cref \
			 -X -x --oformat binary -T../jimpimg.ld
OBJS_KERNEL = main.o gdt.o console.o mpaging.o idt.o isr.o i386.o mm.o kprintf.o \
			  kmalloc.o process.o list.o
OBJS_BOOT = ../boot/head.o ${OBJS_KERNEL}

%.o : %.c
	$(CC) $(CC_OPTIONS) -o$@ $<

%.o : %.S
	$(CC) $(CC_OPTIONS) -o$@ $<
	

all: _build_release $(OBJS_KERNEL) lib_c
	$(LINK) $(LINK_OPTIONS) $(OBJS_BOOT) $(KERNEL_LIBC)  -o kernel.img
	rm -f build.inc


lib_c:
	make -C $(LIBC_PATH)

_build_release:
	@echo "#define BUILD \"$(DATE)\"" > build.inc

run:
	make run -C ..

clean:
	make -C $(LIBC_PATH) clean
	rm -f *.o *.img *.map
	rm -f *~

