myshell: myshell.c
	gcc -o myshell myshell.c
clean:
	rm -f *.o myshell
	echo Clean done