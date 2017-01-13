#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>

#define BUF_SIZE 1024
#define SERVER_PORT 3100

int set_nonblock(int fd);

int is_invalid(int value, std::string err_mes="");

int main(int argc, char **argv);
