myshell: myshell.c
	gcc -o myshell myshell.c -lreadline
clean:
	rm -f *.o myshell
	echo Clean done