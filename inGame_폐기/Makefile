server_exe : inGame.o main.o
	gcc -o server_exe inGame.o main.o -pthread

inGame.o : inGame.c inGame.h
	gcc -c -o inGame.o inGame.c

main.o : main.c main.h
	gcc -c -o main.o main.c -pthread

dummy : dummyClient.c
	gcc -o client dummyClient.c -pthread
