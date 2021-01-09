#
#  _           _  Makefile for jimp image
#   | . |   | |_|  * 11/12/2001  - initial creating
#  \  | | | | |    * 04-nov-2002 - some fixes
#                  * 26-nov-2002 - change structure
#

TARGET=__jimp

KERNEL_PATH = mkernel
BOOT_PATH = boot
BOCHS_JIMP_DIR_PATH = /usr/local/bochs/jimp 

# Debug flags
export DEBUG_FLAGS = #-DKERNEL_DEBUG

# jimp kernel create programm
CREATOR=makejimp

all: tools
	@$(MAKE) -C $(BOOT_PATH)
	@$(MAKE) -C $(KERNEL_PATH)
	@./$(CREATOR) $(BOOT_PATH)/boot.img $(KERNEL_PATH)/kernel.img

tools::
	@$(CC) makejimp.c -o $(CREATOR)

install : tools all
	@echo
	@echo JIMP image installing ... 
	@#@$(MAKEINST) $(INST_OPT) $(TARGET) -d a  
	@#cp $(TARGET) $(BOCHS_JIMP_DIR_PATH)
	@echo done 

run : install 
	bochs -q

clean:
	@rm -f *.map
	@rm -f $(TARGET)
	make -C $(BOOT_PATH) clean
	make -C $(KERNEL_PATH) clean
