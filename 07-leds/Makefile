ARM=y

NAME=hello_leds
obj-m +=$(NAME).o

ifeq ($(ARM), y)
KERNELDIR=/home/blackrose/development-content/micro2440/linux-2.6.32.2
else
KERNELDIR=/usr/src/linux-headers-$(shell uname -r)
endif

APP=app
INSTALL_PATH=/opt/2440rootnfs/root/tmp

all:
	make -C $(KERNELDIR) M=`pwd` modules
	$(CC) $(APP).c -o $(APP)

install:$(APP) $(NAME).ko
	cp $(APP) $(INSTALL_PATH)
	cp $(NAME).ko $(INSTALL_PATH)

PHONY:clean
clean:
	make -C $(KERNELDIR) M=`pwd` modules clean
	rm $(APP)
