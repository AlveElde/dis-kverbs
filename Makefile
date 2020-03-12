obj-m += src/

KVERBS_SRC 		:= ./src
BUS_SRC 		:= $(KVERBS_SRC)/bus
DRV_SRC 		:= $(KVERBS_SRC)/driver
DEV_SRC 		:= $(KVERBS_SRC)/device
KERNEL_BUILD 	:= /lib/modules/$(shell uname -r)/build

all:
	make -C $(KERNEL_BUILD) M=$(PWD) modules

# install:
# 	make -C $(KERNEL_BUILD) M=$(PWD) M=$(PWD) modules_install

clean:
	make -C $(KERNEL_BUILD) M=$(PWD) clean

ins: all
	sudo dmesg -C
	sudo insmod $(BUS_SRC)/dis_bus_mod.ko
	sudo insmod $(DRV_SRC)/dis_driver_mod.ko
	sudo insmod $(DEV_SRC)/dis_device_mod.ko
	dmesg -t

rm:
	sudo rmmod $(DEV_SRC)/dis_device_mod.ko
	sudo rmmod $(DRV_SRC)/dis_driver_mod.ko
	sudo rmmod $(BUS_SRC)/dis_bus_mod.ko
	dmesg -t

