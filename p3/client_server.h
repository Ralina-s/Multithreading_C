#include <iostream>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include <map>
#include <vector>

int set_nonblock(int fd);

int is_invalid(int value, std::string err_mes="");

int create_master_socket(char *ip, int port);

// std::vector<int> read_config(const char* config_name, std::map<int, std::vector<std::pair<char*, int>>> &config_map);

int create_client_socket(int master_socket);

int create_server_socket(char *ip, int port);
