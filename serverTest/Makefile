#서버 exe생성
server_exe : log.o server.o main.o
	gcc -o server_exe log.o server.o main.o

main.o : main.c main.h
	gcc -c -o main.o main.c

server.o : server.c server.h
	gcc -c -o server.o server.c

log.o : log.c log.h
	gcc -c -o log.o log.c

client : dummyClient.c
	gcc -o client dummyClient.c

#dummy client 생성
dummy : dummyClient.c
	gcc -o client dummyClient.c

#실행파일 및 obj파일 삭제
clean :
	rm -f *.o
	rm -f server_exe client
