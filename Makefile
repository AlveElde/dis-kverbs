SRC := ./src

obj-m += dis_kverbs.o
dis_kverbs-objs := $(SRC)/dis_main.o $(SRC)/dis_verbs.o

all:
	make -C /lib/modules/`uname -r`/build M=$(PWD) modules

install:
	make -C /lib/modules/`uname -r`/build M=$(PWD) modules_install

clean:
	make -C /lib/modules/`uname -r`/build M=$(PWD) clean

test:
	sudo dmesg -C
	sudo insmod dis_kverbs.ko
	sudo rmmod dis_kverbs.ko
	dmesg