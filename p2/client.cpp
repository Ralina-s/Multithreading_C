#include "client.h"

int is_invalid(int value, std::string err_mes) {
    if (value < 0) {
    	std::string message = err_mes.length() == 0 ? strerror(errno) : err_mes;
        std::cout << message << std::endl;
        return 1;
    }
    return 0;
}

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

int main(int argc, char **argv)
{
    char buf[BUF_SIZE];

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htons(INADDR_ANY);

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int optval = 1;
    setsockopt(client, 6, SO_REUSEADDR, &optval, sizeof(optval));
    if (is_invalid(connect(client, (struct sockaddr *) &addr, sizeof(addr)))) {
        return -1;
    }

    struct pollfd poll_fd;
    poll_fd.fd = client;
    poll_fd.events = POLLIN;
    while(1) {
        poll(&poll_fd, 1, 1);
        if (poll_fd.revents & POLLIN) {
            int mes_len = recv(poll_fd.fd, buf, sizeof(buf), MSG_NOSIGNAL);
            if (mes_len <= 0) {
                shutdown(client, SHUT_RDWR);
                close(client);
            }
            buf[mes_len] = 0;
            printf("%s", buf);
            fflush(stdout);
        }
        if (fgets(buf, sizeof(buf), stdin)) {
            if (is_invalid(send(poll_fd.fd, buf, strlen(buf), MSG_NOSIGNAL))) {
                return -1;
            }
        }

    }
    shutdown(client, SHUT_RDWR);
    close(client);
    return 0;
}
