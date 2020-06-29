ws:	webserver.o sock_func.o
	gcc -g -o ws webserver.o sock_func.o

webserver.o:	webserver.c sock_func.h
	gcc -c -g webserver.c

sock_func.o:	sock_func.h sock_func.c
	gcc -c -g sock_func.c
