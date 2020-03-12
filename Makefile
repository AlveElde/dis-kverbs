obj-m += dis_bus.o dis_driver.o dis_device.o

SRC := ./src
dis_bus-objs := $(SRC)/dis_bus.o
dis_driver-objs := $(SRC)/dis_driver.o $(SRC)/dis_verbs.o $(SRC)/dis_queue.o
dis_device-objs := $(SRC)/dis_device.o

EXTRA_CFLAGS += -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

ins: all
	sudo dmesg -C
	sudo insmod dis_bus.ko
	sudo insmod dis_driver.ko
	sudo insmod dis_device.ko
	dmesg

rm: all
	sudo rmmod dis_device.ko
	sudo rmmod dis_driver.ko
	sudo rmmod dis_bus.ko
	dmesg

test: ins rm

retest: clean all test