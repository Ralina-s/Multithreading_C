#include "server.h"

int set_nonblock(int fd) {
    int flags;
#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}

int is_invalid(int value, std::string err_mes) {
    if (value < 0) {
        std::string message = err_mes.length() == 0 ? strerror(errno) : err_mes;
        std::cout << message << std::endl;
        return 1;
    }
    return 0;
}

Server::Server() {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htons(INADDR_ANY);

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (is_invalid(server)) {
        exit(-1);
    }
    set_nonblock(server);

    int optval = 1;
    setsockopt(server, 6, SO_REUSEADDR, &optval, sizeof(optval));
    if (is_invalid(bind(server, (struct sockaddr *) &addr, sizeof(addr)))) {
        exit(-1);
    }
    if (is_invalid(listen(server, EPOLL_SIZE))) {
        exit(-1);
    }

    epoll_fd = epoll_create1(0);
    if (is_invalid(epoll_fd)) {
        exit(-1);
    }

    struct epoll_event ev;
    ev.data.fd = server;
    ev.events = EPOLLIN;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server, &ev);
    events = (struct epoll_event*) calloc(EPOLL_SIZE, sizeof(*events));

    struct Args args;
    args.this_class = this;
    pthread_t ttl_thread;
    pthread_create(&ttl_thread, NULL, call_ttl_run, (void*) &args);

    run();
}

void Server::accept_client() {
    int client = accept(server, 0, 0);
    if (is_invalid(client)) {
        exit(-1);
    }
    set_nonblock(client);

    struct epoll_event ev;
    ev.data.fd = client;
    ev.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client, &ev);
    clients.insert(client);
}


void Server::run() {
    while (1) {
        int events_cnt = epoll_wait(epoll_fd, events, EPOLL_SIZE, -1);
        for (int i = 0; i < events_cnt; i++) {
            if(events[i].data.fd == server) {
                accept_client();
            } else {
                memset(buf, 0, sizeof(buf));
                int mes_len = recv(events[i].data.fd, buf, sizeof(buf), MSG_NOSIGNAL);
                if (mes_len <= 0) {
                    shutdown(events[i].data.fd, SHUT_RDWR);
                    close(events[i].data.fd);
                    clients.erase(events[i].data.fd);
                    continue;
                }

                struct Args args;
                args.key = buf + 1;
                args.value = buf + 1 + KEY_SIZE;
                sscanf(buf + 1 + KEY_SIZE + VALUE_SIZE, "%d", &args.time_to_live);
                args.client = events[i].data.fd;
                args.this_class = this;

                pthread_t thread;
                if (buf[0] == SET) {
                    pthread_create(&thread, NULL, call_set, (void*) &args);
                } else if (buf[0] == GET) {
                    pthread_create(&thread, NULL, call_get, (void*) &args);
                } else if (buf[0] == DELETE) {
                    pthread_create(&thread, NULL, call_delete_key, (void*) &args);
                }
            }
        }
    }
}

void* Server::set(void* arg) {
    struct Args* args = (struct Args*) arg;
    std::string key = args->key;
    std::string value = args->value;
    int time_to_live = args->time_to_live;
    int client = args->client;

    int result = key_value.set(key, value, time_to_live);
    std::string mes;

    char buf[KEY_SIZE + VALUE_SIZE + 1];
    if (result == ERROR) {
        buf[0] = result;
        mes = "Ошибка установки ключа: нет места";
    } else if (result == UPDATED || result == ADDED) {
        buf[0] = result;
        if (result == UPDATED) {
            mes = "Обновлено: ";
        } else {
            mes = "Добавлено: ";
        }
        mes += "key = ";
        mes += key;
        mes += ", value = ";
        mes += value;
    }
    memcpy(buf + 1, mes.c_str(), sizeof(buf));
    send(client, buf, sizeof(buf), MSG_NOSIGNAL);
    return NULL;  
}

void* Server::get(void* arg) {
    struct Args* args = (struct Args*) arg;
    std::string key = args->key;
    int client = args->client;

    std::string value;
    int result = key_value.get(key, value);
    std::string mes;

    char buf[KEY_SIZE + VALUE_SIZE + 1];
    if (result == ERROR) {
        buf[0] = result;
        mes = "Ошибка получения значения ";
        mes += key;
        mes += ": ключ не найден";
    } else if (result == UPDATED || result == ADDED) {
        buf[0] = result;
        mes = "Найдено: ";
        mes += "key = ";
        mes += key;
        mes += ", value = ";
        mes += value;
    }
    memcpy(buf + 1, mes.c_str(), sizeof(buf));
    send(client, buf, sizeof(buf), MSG_NOSIGNAL);
    return NULL;  
}

void* Server::delete_key(void* arg) {
    struct Args* args = (struct Args*) arg;
    std::string key = args->key;
    int client = args->client;

    int result = key_value.delete_key(key);
    std::string mes;

    char buf[KEY_SIZE + VALUE_SIZE + 1];
    if (result == ERROR) {
        buf[0] = result;
        mes = "Ошибка удаления ключа ";
        mes += key;
        mes += ": ключ не найден";
    } else if (result == UPDATED || result == ADDED) {
        buf[0] = result;
        mes = "Удалено: ";
        mes += "key = ";
        mes += key;
    }
    memcpy(buf + 1, mes.c_str(), sizeof(buf));
    send(client, buf, sizeof(buf), MSG_NOSIGNAL);
    return NULL;  
}

void* Server::ttl_run(void* arg) {
    while (1) {
        sleep(60);
        key_value.delete_all_ttl();
    }
    return NULL;
}

Server::~Server() {
    shutdown(server, SHUT_RDWR);
    if (is_invalid(close(server))) {
        exit(-1);
    }
}

void* call_set(void* arg) {
    struct Args*  args = (struct Args*) arg;
    (Server*)(args->this_class)->set(arg);
    return NULL;
}

void* call_get(void* arg) {
    struct Args*  args = (struct Args*) arg;
    (Server*)(args->this_class)->get(arg);
    return NULL;
}

void* call_delete_key(void* arg) {
    struct Args*  args = (struct Args*) arg;
    (Server*)(args->this_class)->delete_key(arg);
    return NULL;
}

void* call_ttl_run(void* arg) {
    struct Args*  args = (struct Args*) arg;
    (Server*)(args->this_class)->ttl_run(arg);
    return NULL;
}
