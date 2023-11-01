DEP = tcp.o

target: clean client_tcp server_tcp reqFileClient fileServer

client_tcp: client_tcp.c $(DEP)
	gcc -o client_tcp.exe client_tcp.c tcp.o

server_tcp: server_tcp.c $(DEP)
	gcc -o server_tcp.exe server_tcp.c tcp.o

reqFileClient: reqFileClient.c tcp.o
	gcc -o reqFileClient.exe reqFileClient.c tcp.o

fileServer: fileServer.c tcp.o
	gcc -o fileServer.exe fileServer.c tcp.o

$(DEP): tcp.h
	gcc -c tcp.c

clean:
	rm -f *.o
	rm -f net/*
	rm -f clientSide/system.fifo
	rm -f serverSide/system.fifo
	rm -f clientSide/ports/*
	rm -f serverSide/ports/*