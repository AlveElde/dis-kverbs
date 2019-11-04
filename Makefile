obj-m += dis_kverbs.o
dis_kverbs-objs := src/dis_main.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
	sudo dmesg -C
	sudo insmod dis_kverbs.ko
	sudo rmmod dis_kverbs.ko
	dmesg