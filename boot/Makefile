#
#  _           _   Makefile for jimp image assembly
#   | . |   | |_|  08/09/2001
#  \  | | | | |  
#


INCLUDE_PATH=../include

# nasm options for boot.asm and head.asm
ASM=nasm
ASM_OPTIONS_BOOT=-f bin
ASM_OPTIONS_COFF=-f elf 

all:
	make boot
	make head

#boot: boot.asm
#		@$(ASM) $(ASM_OPTIONS_BOOT) boot.asm -o boot.img
		
boot: boot.S
	@gcc boot.S -c -nostdlib -nostartfiles -nodefaultlibs
	@ld boot.o -s --oformat binary  -o boot.img -Ttext 0x0000 

head: head.S
	gcc head.S -c -o head.o

clean:
	@rm -f *.o *.lst *.img
