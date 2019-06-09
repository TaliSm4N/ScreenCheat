server_exe : inGame.o log.o main.o
	gcc -pthread -o server_exe log.o inGame.o main.o

clean : server_exe
	rm -rf server_exe *.o

log.o : log.c log.h
	gcc -c -o log.o log.c

inGame.o : inGame.c inGame.h
	gcc -pthread -c -o inGame.o inGame.c

main.o : main.c main.h
	gcc -c -o main.o main.c
