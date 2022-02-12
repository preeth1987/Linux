ifeq ($(KERNELRELEASE),)

KERNELDIR := ${LINUX_HEADERS}

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) C=1 modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) C=1 clean

.PHONY: module clean

else

MODULE = br_pkt_inspect.o
CFLAGS_$(MODULE) := -DDEBUG
obj-m := $(MODULE)

endif
