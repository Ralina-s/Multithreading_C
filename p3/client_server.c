#include "client_server.h"

int set_nonblock(int fd) {
    int flags;
#if defined(O_NONBLOCK)
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
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

int create_master_socket(char *ip, int port) {
    int master_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (is_invalid(master_socket)) {
    	return -1;
    }
    set_nonblock(master_socket);

    struct sockaddr_in sock_adr;
    memset(&sock_adr, 0, sizeof(sock_adr));
    sock_adr.sin_family = AF_INET;
    sock_adr.sin_port = htons(port);
    inet_aton(ip, &sock_adr.sin_addr);
    int optval = 1;
    if (is_invalid(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR,
                       &optval, sizeof(optval))) ||
    	is_invalid(bind(master_socket, (struct sockaddr *) &sock_adr,
             sizeof(sock_adr)) ||
    	is_invalid(listen(master_socket, 0)))) {
        return -1;
    }
    return master_socket;
}

int create_client_socket(int master_socket) {
    int client_socket = accept(master_socket, 0, 0);
    if (client_socket >= 0)
        set_nonblock(client_socket);
    else
        std::cout << strerror(errno) << std::endl;
    return client_socket;
}

int create_server_socket(char *ip, int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (is_invalid(server_socket)) {
        return -1;
    }

    struct sockaddr_in sock_adr;
    memset(&sock_adr, 0, sizeof(sock_adr));
    sock_adr.sin_family = AF_INET;
    sock_adr.sin_port = htons(port);
    inet_aton(ip, &sock_adr.sin_addr);
    if (is_invalid(connect(server_socket, (struct sockaddr*)&sock_adr, sizeof(sock_adr)))) {
        close(server_socket);
        return -1;
    }
    return server_socket;
}
