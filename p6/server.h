#include "Key-Value.h"

// typedef struct Args Args;

// int set_nonblock(int fd);

// int is_invalid(int value, std::string err_mes="");

void* call_set(void*);
void* call_get(void*);
void* call_delete_key(void*);
void* call_ttl_run(void* arg);

class Server {
    Key_Value key_value;
    char buf[KEY_SIZE + VALUE_SIZE + 1];

    int server;
    std::set <int> clients;

    int epoll_fd;
    struct epoll_event *events;

    void run();
    void accept_client();

public:
    Server();
    ~Server();

    void* set(void* arg);
    void* get(void* arg);
    void* delete_key(void* arg);
    void* ttl_run(void* arg);
};

struct Args {
    std::string key;
    std::string value;
    int time_to_live;
    int client;
    Server* this_class;
};
