server_exe : log.o main.o
	gcc -o server_exe log.o main.o

log.o : log.c log.h
	gcc -c -o log.o log.c

main.o : main.c main.h
	gcc -c -o main.o main.c

dummy.o : dummyClient.c main.h
	gcc -c -o dummy.o dummyClient.c

client_exe : dummy.o log.o
	gcc -o client_exe log.o dummy.o
