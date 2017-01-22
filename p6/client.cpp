#include "client.h"

Client::Client() {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htons(INADDR_ANY);

    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // int optval = 1;
    // setsockopt(client, 6, SO_REUSEADDR, &optval, sizeof(optval));
    if (is_invalid(connect(client, (struct sockaddr *) &addr, sizeof(addr)))) {
        exit(-1);
    }
    set_nonblock(client);

    poll_fd.fd = client;
    poll_fd.events = POLLIN;
}

Client::~Client() {
    shutdown(client, SHUT_RDWR);
    close(client);
}

void Client::receive() {
    while (1) {
        sleep(1);
        poll(&poll_fd, 1, 1);
        if (poll_fd.revents & poll_fd.events) {
                // std::cout << "client: получила событие, которое ждала" << std::endl << std::flush;
            int mes_len = recv(poll_fd.fd, buf, BUF_SIZE, 0);
            if (mes_len <= 0) {
                shutdown(client, SHUT_RDWR);
                close(client);
            }
            // buf[mes_len] = 0;
            return;
        }
    }
}

int Client::set(std::string key, std::string value, int ttl) {

    buf[0] = SET;
    memcpy(buf + 1, key.c_str(), KEY_SIZE);
    memcpy(buf + 1 + KEY_SIZE, value.c_str(), VALUE_SIZE);
    memcpy(buf + 1 + KEY_SIZE + VALUE_SIZE, &ttl, TTL_SIZE);

    if (is_invalid(send(poll_fd.fd, buf, BUF_SIZE, 0))) {
            // std::cout << "client: не получилось отправить" << std::endl << std::flush;
        return -1;
    }   

    // std::cout << "client: перед ресив" << std::endl << std::flush;
    receive();
    // std::cout << "client: после ресив" << std::endl << std::flush;

    printf("%s\n", buf + 1);
    fflush(stdout);
    if (buf[0] == ERROR) {
        return -1;
    }
    memset(buf, 0, BUF_SIZE);
    return 0;
}

int Client::get(std::string key, std::string return_value) {

    buf[0] = GET;
    memcpy(buf + 1, key.c_str(), KEY_SIZE);

    if (is_invalid(send(poll_fd.fd, buf, strlen(buf), MSG_NOSIGNAL))) {
        return -1;
    }

    receive();

    printf("%s\n", buf + 1);
    fflush(stdout);
    if (buf[0] == ERROR) {
        return -1;
    }
    memset(buf, 0, BUF_SIZE);
    return 0;
}

int Client::delete_key(std::string key) {

    buf[0] = DELETE;
    memcpy(buf + 1, key.c_str(), KEY_SIZE);

    if (is_invalid(send(poll_fd.fd, buf, strlen(buf), MSG_NOSIGNAL))) {
        return -1;
    }

    receive();

    printf("%s\n", buf + 1);
    fflush(stdout);
    if (buf[0] == ERROR) {
        return -1;
    }
    memset(buf, 0, BUF_SIZE);
    return 0;
}
