obj-m += dis_scilib_if_mod.o
dis_scilib_if_mod-objs := dis_scilib_if.o dis_msq.o

KVERBS_SRC 		:= $(obj)/..
DIS_SRC			:= /opt/DIS
SCI_SRC			:= /home/alve/scilib
KERNEL_SRC		:= /home/alve/kernel
OLD_DIS_SRC		:= /home/alve/royn-lts/DIS/src

EXTRA_CFLAGS 			+= -DDEBUG
EXTRA_CFLAGS 			+= -I$(KVERBS_SRC)/common

CPPFLAGS += -D_DIS_KERNEL_ -DOS_IS_LINUX -DCPU_ADDR_IS_64_BIT 	\
            -DLINUX -DUNIX -DCPU_ARCH_IS_X86_64 -DADAPTER_IS_PX \
			-DINTERFACE_IS_GENIF -DDEBUG -DHAVE_WAIT_QUEUE_ENTRY

EXTRA_CFLAGS += .                                               	\
				-I$(DIS_SRC)/include								\
				-I$(DIS_SRC)/include/dis							\
				-I$(DIS_SRC)/include/os								\
				-I$(SCI_SRC)/.										\
				-I$(SCI_SRC)/GENIF									\
				-I$(SCI_SRC)/GENIF/LINUX							\
				-I$(SCI_SRC)/GENIF/LINUX/os							\
				-I$(SCI_SRC)/SISCI									\
				-I$(KERNEL_SRC)/.               					\
				-I$(KERNEL_SRC)/include       						\
				-I$(KERNEL_SRC)/include/LINUX 						\
				-I$(OLD_DIS_SRC)/IRM_GX/drv/src                   	\
				-I$(OLD_DIS_SRC)/IRM_GX/drv/include               	\
				-I$(OLD_DIS_SRC)/IRM_GX/drv/src/LINUX             	\
				-I$(OLD_DIS_SRC)/IRM_GX/drv/src/LINUX/os			\
				-I$(OLD_DIS_SRC)/include                   			\
				-I$(OLD_DIS_SRC)/include/dis              			\

EXTRA_CFLAGS += ${CPPFLAGS}
