obj-m += test_mwatch.o
obj-m += mwatch.o

.PHONY : all clean

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

