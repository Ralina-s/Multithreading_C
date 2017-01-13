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

int main(int argc, char * argv[])
{
    char msg_buf[BUF_SIZE];

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htons(INADDR_ANY);

    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (is_invalid(listener)) {
        return -1;
    }
    set_nonblock(listener);

    int optval = 1;
    setsockopt(listener, 6, SO_REUSEADDR, &optval, sizeof(optval));
    if (is_invalid(bind(listener, (struct sockaddr *) &addr, sizeof(addr)))) {
        return -1;
    }
    if (is_invalid(listen(listener, EPOLL_SIZE))) {
        return -1;
    }

    int epoll_fd = epoll_create1(0);
    if (is_invalid(epoll_fd)) {
        return -1;
    }

    struct epoll_event ev;
    ev.data.fd = listener;
    ev.events = EPOLLIN;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener, &ev);

    std::set <int> clients;
    struct epoll_event *events = (struct epoll_event*) calloc(EPOLL_SIZE, sizeof(*events));

    while (1) {
        int events_cnt = epoll_wait(epoll_fd, events, EPOLL_SIZE, -1);
        for (int i = 0; i < events_cnt; i++) {
            if(events[i].data.fd == listener) {
                int client = accept(listener, 0, 0);
                if (is_invalid(client)) {
                    return -1;
                }
                set_nonblock(client);

                fprintf(stdout, accept_log);
                fflush(stdout);

                struct epoll_event ev;
                ev.data.fd = client;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client, &ev);
                clients.insert(client);
                send(client, welcome_msg, sizeof(welcome_msg), MSG_NOSIGNAL);
            } else {
                memset(msg_buf, 0, sizeof(msg_buf));
                int received = recv(events[i].data.fd, msg_buf, sizeof(msg_buf), MSG_NOSIGNAL);
                if (received <= 0) {
                    fprintf(stdout, terminate_log);
                    fflush(stdout);
                    shutdown(events[i].data.fd, SHUT_RDWR);
                    close(events[i].data.fd);
                    clients.erase(events[i].data.fd);
                    continue;
                }
                msg_buf[received] = 0;
                fprintf(stdout, "%s", msg_buf);
                fflush(stdout);
                auto p = clients.begin();
                for (; p != clients.end(); p++) {
                    send(*p, msg_buf, received, MSG_NOSIGNAL);
                }
            }
        }
    }
    shutdown(listener, SHUT_RDWR);
    if (is_invalid(close(listener))) {
        return -1;
    }
    return 0;
}
