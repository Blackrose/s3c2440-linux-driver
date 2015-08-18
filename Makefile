
obj-m +=mem.o

all:
	make -C /usr/src/kernels/2.6.35.14-106.fc14.i686.PAE M=`pwd` modules
	gcc main.c -o main

PHONY:clean

clean:
	make -C /usr/src/kernels/2.6.35.14-106.fc14.i686.PAE M=`pwd` modules clean
	rm -rf main

