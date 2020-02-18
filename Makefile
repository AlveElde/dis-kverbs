obj-m += dis_kverbs.o

SRC := ./src
dis_kverbs-objs := $(SRC)/dis_main.o $(SRC)/dis_verbs.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

verbs:
	sudo dmesg -C
	sudo insmod dis_kverbs.ko
	sudo rmmod dis_kverbs.ko
	dmesg