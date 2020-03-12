obj-m += dis_driver_mod.o
dis_driver_mod-objs := dis_driver.o dis_verbs.o dis_queue.o

KVERBS_SRC 	:= $(obj)/..

EXTRA_CFLAGS += -DDEBUG
EXTRA_CFLAGS += -I$(KVERBS_SRC)/common
