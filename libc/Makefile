#
#  _           _   Makefile for kernel library (libc)
#   | . |   | |_|  19-jul-2005
#  \  | | | | |  
#

NAME=libc.a

# gcc compiler
INCLUDE_PATH=../include
CC=gcc
CC_OPTIONS= $(DEBUG_FLAGS) -O2 -c -s -I$(INCLUDE_PATH) -nostdlib -nostdinc++ \
			-nostdinc -fno-builtin -fomit-frame-pointer -Wimplicit \
			-fwritable-strings -ffreestanding
# archiver
ARC = ar
ARC_OPTIONS = r
LIBC_OBJS = kmemset.o kmemmove.o ksetjmp.o kmemcpy.o

%.o : %.c
	$(CC) $(CC_OPTIONS) -o$@ $<

%.o : %.S
	$(CC) $(CC_OPTIONS) -o$@ $<

all: $(LIBC_OBJS)
	$(ARC) $(ARC_OPTIONS) $(NAME) $(LIBC_OBJS)

clean:
	@rm -f *.o *.map *.a *~

