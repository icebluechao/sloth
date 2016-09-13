ifneq ($(KERNELRELEASE),)

obj-m += sloth.o
sloth-objs := main.o hook.o tcp_flow.o

else

KDIR ?= /lib/modules/$(shell uname -r)/build
MAKE = make

all:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules clean

sparse:
	@$(MAKE) C=2 -C $(KDIR) M=$(PWD)

endif
