all: proxy
proxy: proxy.cpp client_server.c reciever.c
	g++ -o proxy proxy.cpp client_server.c reciever.c -g -std=c++11 -lev -Wall -Werror
clean:
	rm -rf proxy *.tmp
