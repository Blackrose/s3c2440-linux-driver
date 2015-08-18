ARM=y

MODULE=hello-driver
obj-m +=$(MODULE).o

ifeq ($(ARM), y)
KERNELDIR=/home/blackrose/development-content/micro2440/linux-2.6.32.2
else
KERNELDIR=/usr/src/linux-headers-$(shell uname -r)
endif

INSTALL_DIR=/opt/2440rootnfs/root/tmp

all:
	make -C $(KERNELDIR) M=`pwd` modules
	$(CC) app.c -o app

install:
	cp $(MODULE).ko $(INSTALL_DIR)
	cp app $(INSTALL_DIR)

PHONY:clean
clean:
	make -C $(KERNELDIR) M=`pwd` modules clean
	rm -rf app

