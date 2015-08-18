ARM=y
obj-m:=hello-driver.o

ifeq ($(ARM), y)
KERNELDIR=/home/blackrose/development-content/micro2440/linux-2.6.32.2
else
KERNELDIR=/usr/src/linux-headers-$(shell uname -r)
endif

all:
	make -C $(KERNELDIR) M=`pwd` modules

PHONY:clean

clean:
	make -C $(KERNELDIR) M=`pwd` modules clean

