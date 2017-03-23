obj-m += processInfo.o

myshell: myshell.c
	gcc -o myshell myshell.c -lreadline
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.o myshell
	echo Clean done