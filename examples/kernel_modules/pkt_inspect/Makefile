ifeq ($(KERNELRELEASE),)

KERNELDIR := ${LINUX_HEADERS}
#KERNELDIR := /projects/slxos_dev/ps407411/SLX2/slxos_main/build/swbd4000/linux_4_9

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) C=1 modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) C=1 clean

.PHONY: module clean

else

MODULE = ipv6_pkt_inspect.o
CFLAGS_$(MODULE) := -DDEBUG
obj-m := $(MODULE)

endif
