#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <list>
#include <time.h>
#include <netinet/in.h>
#include <errno.h>
#include <set>
#include <stdlib.h>
#include <string.h>
#include <map>

#define BUF_SIZE 1024
#define SERVER_PORT 3100
#define EPOLL_SIZE 1024

const char welcome_msg[] = "Welcome!\n";
const char accept_log[] = "accepted connection\n";
const char terminate_log[] = "connection terminated\n";

int set_nonblock(int fd);

int is_invalid(int value, std::string err_mes="");

int main(int argc, char * argv[]);
