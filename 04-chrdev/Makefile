ARM=y

MODULE_NAME=hello-driver
obj-m +=$(MODULE_NAME).o

INSTALL_DIR=/opt/2440rootnfs/root/tmp

ifeq ($(ARM), y)
KERNELDIR=/home/blackrose/development-content/micro2440/linux-2.6.32.2
else
KERNELDIR=/usr/src/linux-headers-$(shell uname -r)
endif

all:
	make -C $(KERNELDIR) M=`pwd` modules
install:
	cp $(MODULE_NAME).ko $(INSTALL_DIR)
PHONY:clean
clean:
	make -C $(KERNELDIR) M=`pwd` modules clean

