ARM=y

#DEV=dev_leds
#DRV=micro2440_backlight
DRV=mini2440_backlight
#obj-m +=$(DEV).o
obj-m +=$(DRV).o

ifeq ($(ARM), y)
KERNELDIR=${HOME}/micro2440/linux-2.6.32.2
else
KERNELDIR=/usr/src/linux-headers-$(shell uname -r)
endif

APP=app
INSTALL_PATH=/opt/2440rootnfs/root/tmp

all:
	make -C $(KERNELDIR) M=`pwd` modules
	#$(CC) $(APP).c -o $(APP)

install:$(APP) $(DRV).ko
	cp $(APP) $(INSTALL_PATH)
	cp $(DRV).ko $(INSTALL_PATH)

PHONY:clean
clean:
	make -C $(KERNELDIR) M=`pwd` modules clean
	rm -rf $(APP)
