ifneq ($(KERNELRELEASE),)

#EXTRA_CFLAGS += -g -O1 -DDEBUG=1
#EXTRA_CFLAGS += -g -O3

obj-m += sloth.o
sloth-objs := main.o

else

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	@make -C $(KDIR) M=$(PWD) modules

clean:
	@make -C $(KDIR) M=$(PWD) modules clean
	@rm -rf modules order

endif
