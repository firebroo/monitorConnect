obj-m := jprobe.o
 
PWD       := $(shell pwd)
 
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
 
clean:
	rm -rf *.ko *.order *.symvers *.o *~ core .*.cmd *.mod.c ./tmp_version
