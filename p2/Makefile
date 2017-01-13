all: server client
server: server.cpp
	g++ -std=c++11 -o server server.cpp -Wall -Werror
client: client.cpp
	g++ -std=c++11 -o client client.cpp -Wall -Werror
clean:
	rm -rf server
	rm -rf client
