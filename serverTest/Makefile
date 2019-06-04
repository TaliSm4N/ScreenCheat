#서버 exe생성
server_exe : log.o server.o main.o database.o
	gcc -o server_exe log.o server.o main.o database.o -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient

main.o : main.c main.h
	gcc -c -o main.o main.c

server.o : server.c server.h
	gcc -c -o server.o server.c

log.o : log.c log.h
	gcc -c -o log.o log.c

database.o : database.c database.h
	gcc -c -o database.o database.c -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient

client : dummyClient.c
	gcc -o client dummyClient.c

#dummy client 생성
dummy : dummyClient.c
	gcc -o client dummyClient.c

#실행파일 및 obj파일 삭제
clean :
	rm -f *.o
	rm -f server_exe client
