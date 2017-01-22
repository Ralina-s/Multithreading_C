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
#include <sys/epoll.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>

#include <string.h>
#include <set>
#include <list>
#include <map>
#include <vector>

#define MAX_KEY  1000

#define KEY_SIZE 1024
#define VALUE_SIZE 1024
#define SERVER_PORT 3100
#define EPOLL_SIZE 1024

#ifndef CONSTANT
#define CONSTANT 

int set_nonblock(int fd);

int is_invalid(int value, std::string err_mes="");

enum {
    UPDATED,
    ADDED,
    FOUND,
    DELETED,
    ERROR
};

enum {
    SET,
    GET,
    DELETE
};
#endif