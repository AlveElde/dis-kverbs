obj-m += src/

INSTALL_DIR		:= $(HOME)
SRC 			:= ./src
BUS_SRC 		:= $(SRC)/bus
DRV_SRC 		:= $(SRC)/driver
DEV_SRC 		:= $(SRC)/device
SCI_SRC			:= $(INSTALL_DIR)/DIS/src/SCI_SOCKET/scilib/GENIF/LINUX
KERNEL_BUILD 	:= /lib/modules/$(shell uname -r)/build

SCI_SYMBOLS 			:= $(SCI_SRC)/Module.symvers
KBUILD_EXTRA_SYMBOLS 	+= $(SCI_SYMBOLS)

SCI_IF_REQ_PARAMS = local_adapter_no=0 remote_node_id=4 is_initiator=N use_l_qpn=Y
SCI_IF_RES_PARAMS = local_adapter_no=0 remote_node_id=8 is_initiator=Y use_l_qpn=Y

.PHONY = all req res rm req-r res-r rm-srp clean

all:
	make -C $(KERNEL_BUILD) M=$(PWD) modules

clean:
	make -C $(KERNEL_BUILD) M=$(PWD) clean

dmesg-c:
	sudo dmesg -C

rm-srp:
	sudo rmmod ib_srp
	sudo rmmod ib_srpt

req: dmesg-c all
	sudo insmod $(SCI_SRC)/dis_msq.ko
	sudo insmod $(DRV_SRC)/dis_sci_if_mod.ko $(SCI_IF_REQ_PARAMS)
	sudo insmod $(BUS_SRC)/dis_bus_mod.ko
	sudo insmod $(DRV_SRC)/dis_driver_mod.ko
	sudo insmod $(DEV_SRC)/dis_device_mod.ko
	dmesg -t

res: dmesg-c all
	sudo insmod $(SCI_SRC)/dis_msq.ko
	sudo insmod $(DRV_SRC)/dis_sci_if_mod.ko $(SCI_IF_RES_PARAMS)
	sudo insmod $(BUS_SRC)/dis_bus_mod.ko
	sudo insmod $(DRV_SRC)/dis_driver_mod.ko
	sudo insmod $(DEV_SRC)/dis_device_mod.ko
	dmesg -t

rm: dmesg-c
	sudo rmmod dis_device_mod.ko
	sudo rmmod dis_driver_mod.ko
	sudo rmmod dis_bus_mod.ko
	sudo rmmod dis_sci_if_mod.ko
	sudo rmmod dis_msq.ko
	dmesg -t

req-r: rm all req

res-r: rm all res