obj-m += dis_driver_mod.o
dis_driver_mod-objs := dis_driver.o dis_verbs.o dis_qp.o

obj-m += dis_sci_if_mod.o
dis_sci_if_mod-objs := dis_sci_if.o

INSTALL_DIR		:= $(HOME)
KVERBS_SRC 		:= $(obj)/..
OPT_DIS_SRC		:= /opt/DIS
DIS_SRC			:= $(INSTALL_DIR)/DIS/src
SCILIB_SRC		:= $(DIS_SRC)/SCI_SOCKET/scilib
OSIF			:= $(DIS_SRC)/COMMON/osif/kernel

EXTRA_CFLAGS += -D_DIS_KERNEL_ 			\
				-DOS_IS_LINUX 			\
				-DCPU_ADDR_IS_64_BIT 	\
            	-DLINUX 				\
				-DUNIX 					\
				-DCPU_ARCH_IS_X86_64 	\
				-DADAPTER_IS_PX 		\
				-DINTERFACE_IS_GENIF 	\
				-DHAVE_WAIT_QUEUE_ENTRY	\
				# -DDIS_USE_DMA			\
				# -DDEBUG 				\

EXTRA_CFLAGS += -I$(KVERBS_SRC)/common						\
				-I$(OPT_DIS_SRC)/include						\
				-I$(OPT_DIS_SRC)/include/dis					\
				-I$(OPT_DIS_SRC)/include/os						\
				-I$(SCILIB_SRC)/.								\
				-I$(SCILIB_SRC)/GENIF							\
				-I$(SCILIB_SRC)/GENIF/LINUX					\
				-I$(SCILIB_SRC)/GENIF/LINUX/os					\
				-I$(SCILIB_SRC)/SISCI							\
				-I$(OSIF)/.               			\
				-I$(OSIF)/include       				\
				-I$(OSIF)/include/LINUX 				\
				-I$(DIS_SRC)/IRM_GX/drv/src             \
				-I$(DIS_SRC)/IRM_GX/drv/include         \
				-I$(DIS_SRC)/IRM_GX/drv/src/LINUX       \
				-I$(DIS_SRC)/IRM_GX/drv/src/LINUX/os	\
				-I$(DIS_SRC)/include                   	\
				-I$(DIS_SRC)/include/dis              	\


