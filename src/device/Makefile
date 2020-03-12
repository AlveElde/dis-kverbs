obj-m += dis_device_mod.o
dis_device_mod-objs := dis_device.o

KVERBS_SRC 	:= $(obj)/..

EXTRA_CFLAGS += -DDEBUG
EXTRA_CFLAGS += -I$(KVERBS_SRC)/common