ARM=y

#DEV=dev_leds
DRV=s3c2440wdt
#obj-m +=$(DEV).o
obj-m +=$(DRV).o

ifeq ($(ARM), y)
KERNELDIR=/home/blackrose/micro2440/linux-2.6.32.2
else
KERNELDIR=/usr/src/linux-headers-$(shell uname -r)
endif

APP=app
INSTALL_PATH=/opt/2440rootnfs/root/tmp

all:
	make -C $(KERNELDIR) M=`pwd` modules
	$(CC) $(APP).c -o $(APP)

install:$(APP) $(DRV).ko
	cp $(APP) $(INSTALL_PATH)
	cp $(DRV).ko $(INSTALL_PATH)

PHONY:clean
clean:
	make -C $(KERNELDIR) M=`pwd` modules clean
	rm -rf $(APP)
