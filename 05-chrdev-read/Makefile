ARM=y
MODULE=hello-driver
obj-m +=$(MODULE).o


INSTALL_DIR=/opt/2440rootnfs/root/tmp

APP=app

ifeq ($(ARM), y)
KERNELDIR=/home/blackrose/development-content/micro2440/linux-2.6.32.2
else
KERNELDIR=/usr/src/linux-headers-$(shell uname -r)
endif


all:
	make -C $(KERNELDIR) M=`pwd` modules
	$(CC) $(APP).c -o $(APP)
install:
	cp $(MODULE).ko $(INSTALL_DIR)
	@echo "you must run next code when you write a user application to visit this driver"
	@echo "mknod /dev/hello-driver c <MAJOR> 0"
	cp $(APP) $(INSTALL_DIR)
PHONY:clean
clean:
	make -C $(KERNELDIR) M=`pwd` modules clean
	rm -rf $(APP)

