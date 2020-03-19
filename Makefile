obj-m += src/

SRC 			:= ./src
BUS_SRC 		:= $(SRC)/bus
DRV_SRC 		:= $(SRC)/driver
DEV_SRC 		:= $(SRC)/device
SCI_IF_SRC 		:= $(SRC)/sci_if
SCI_SRC			:= /home/alve/scilib/GENIF/LINUX
KERNEL_BUILD 	:= /lib/modules/$(shell uname -r)/build

SCI_SYMBOLS 			:= $(SCI_SRC)/Module.symvers
KBUILD_EXTRA_SYMBOLS 	+= $(SCI_SYMBOLS)

all:
	make -C $(KERNEL_BUILD) M=$(PWD) modules

# install:
# 	make -C $(KERNEL_BUILD) M=$(PWD) M=$(PWD) modules_install

clean:
	make -C $(KERNEL_BUILD) M=$(PWD) clean

dmesg-c:
	sudo dmesg -C

rm-srp:
	sudo rmmod ib_srp
	sudo rmmod ib_srpt

req: dmesg-c
	sudo insmod $(SCI_SRC)/dis_msq.ko
	sudo insmod $(SCI_IF_SRC)/sci_if_mod.ko local_adapter_no=0 remote_node_id=4 is_initiator=N
	sudo insmod $(BUS_SRC)/dis_bus_mod.ko
	sudo insmod $(DRV_SRC)/dis_driver_mod.ko
	sudo insmod $(DEV_SRC)/dis_device_mod.ko
	dmesg -t

res: dmesg-c
	sudo insmod $(SCI_SRC)/dis_msq.ko
	sudo insmod $(SCI_IF_SRC)/sci_if_mod.ko local_adapter_no=0 remote_node_id=8 is_initiator=Y
	sudo insmod $(BUS_SRC)/dis_bus_mod.ko
	sudo insmod $(DRV_SRC)/dis_driver_mod.ko
	sudo insmod $(DEV_SRC)/dis_device_mod.ko
	dmesg -t

rm: dmesg-c
	sudo rmmod dis_device_mod.ko
	sudo rmmod dis_driver_mod.ko
	sudo rmmod dis_bus_mod.ko
	sudo rmmod sci_if_mod.ko
	sudo rmmod dis_msq.ko
	dmesg -t

req-r: rm all req

res-r: rm all res