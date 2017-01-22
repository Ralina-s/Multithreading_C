#include "constants.h"

// int set_nonblock(int fd);

// int is_invalid(int value, std::string err_mes="");

class Client {
    int client;
    struct pollfd poll_fd;

    char buf[BUF_SIZE];  

    void receive();

public:
    Client();
    ~Client();

    int set(std::string key, std::string value, int ttl);
    int get(std::string key, std::string return_value);
    int delete_key(std::string key);
};
