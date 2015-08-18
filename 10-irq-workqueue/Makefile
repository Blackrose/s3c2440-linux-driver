ARM=y
DRIVER=hello_buttons
obj-m +=$(DRIVER).o

ifeq ($(ARM), y)
KERNELDIR=/home/blackrose/development-content/micro2440/linux-2.6.32.2
else
KERNELDIR=/usr/src/linux-headers-$(shell uname -r)
endif

INSTALLDIR=/opt/2440rootnfs/root/tmp

all:
	make -C $(KERNELDIR) M=`pwd` modules
	$(CC) app.c -o app
install:
	cp $(DRIVER).ko $(INSTALLDIR)
	cp app $(INSTALLDIR)
PHONY:clean
clean:
	make -C $(KERNELDIR) M=`pwd` modules clean
	rm -rf app

