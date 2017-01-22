#include "constants.h"

// int set_nonblock(int fd);

// int is_invalid(int value, std::string err_mes="");

class Client {
    int client;
    struct pollfd poll_fd;

    char buf[KEY_SIZE + VALUE_SIZE + 1];    // type, key, value

    void receive();

public:
    Client();
    ~Client();

    int set(std::string key, std::string value);
    int get(std::string key, std::string return_value);
    int delete_key(std::string key);
};
