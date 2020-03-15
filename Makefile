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
	
dmesg-p:
	dmesg -t


## Insmod SCI ##
ins-dis_msq:
	sudo insmod $(SCI_SRC)/dis_msq.ko

ins-req-sci_if: ins-dis_msq
	sudo insmod $(SCI_IF_SRC)/sci_if_mod.ko local_adapter_no=0 remote_node_id=4 is_initiator=N

ins-res-sci_if: ins-dis_msq
	sudo insmod $(SCI_IF_SRC)/sci_if_mod.ko local_adapter_no=0 remote_node_id=4 is_initiator=Y

ins-req: dmesg-c ins-req-sci_if dmesg-p

ins-res: dmesg-c ins-res-sci_if dmesg-p

## Rmmod SCI ##
rm-sci_if:
	sudo rmmod sci_if_mod.ko

rm-dis_msq: rm-sci_if
	sudo rmmod dis_msq.ko

rm-sci: dmesg-c rm-dis_msq dmesg-p

## Insmod DIS ## 
ins-dis_bus:
	sudo insmod $(BUS_SRC)/dis_bus_mod.ko

ins-dis_driver: ins-dis_bus
	sudo insmod $(DRV_SRC)/dis_driver_mod.ko

ins-dis_device: ins-dis_driver
	sudo insmod $(DEV_SRC)/dis_device_mod.ko

ins-dis: dmesg-c ins-dis_device dmesg-p

## Rmmod DIS ##
rm-dis_device:
	sudo rmmod dis_device_mod.ko

rm-dis_driver: rm-dis_device
	sudo rmmod dis_driver_mod.ko

rm-dis_bus: rm-dis_driver
	sudo rmmod dis_bus_mod.ko

rm-dis: dmesg-c rm-dis_bus dmesg-p

