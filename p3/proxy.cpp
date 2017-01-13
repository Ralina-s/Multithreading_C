#include <stdio.h>
#include <iostream>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include <ev.h>

#include <map>
#include <vector>

#include "reciever.h"
#include "client_server.h"

std::map<int, std::vector<std::pair<char*, int>>> config_map;
std::map<int, reciever*> sender_reciever;
std::map<int, int> senders_map;

const int SIZE_LEN = 1024;
const int SIZE_IP = 32;
const int COUNT_PORTS = 10;
const int BUFFER_SIZE = 1024;
static char localhost[] = "127.0.0.1";

std::vector<int> read_config(const char* config_name,
                             std::map<int, std::vector<std::pair<char*, int>>> &config_map) {
    char buf_config[SIZE_LEN];
    std::vector<int> master_sockets;

    FILE *config = fopen(config_name, "r");
    int port;
    std::cout << "Config file:" << std::endl;
    while(fgets(buf_config, sizeof(buf_config), config) != NULL) {
        int n = 0;
        sscanf(buf_config, "%d %n", &port, &n);
        n += 1;
        std::cout << port;

        int master_socket = create_master_socket(localhost, port);
        config_map[master_socket] = std::vector<std::pair<char*, int>>();
        int new_n;
        char ip[SIZE_IP];
        while (sscanf(buf_config + n, "%s : %d %n", ip, &port, &new_n) == 2) {
            std::cout << ", ";
            n += new_n + 1;
            std::cout << ip << ':' << port;
            config_map[master_socket].push_back(std::make_pair(ip, port));
        }
        std::cout << std::endl;
        master_sockets.push_back(master_socket);
    }
    std::cout << "End of config file" << std::endl;
    fclose(config);
    return master_sockets;
}

void write_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    int fd = senders_map[watcher->fd];
    reciever *rec = sender_reciever[fd];
    int mes_len = reciever_write(rec, BUFFER_SIZE);
    if (mes_len == -1 || (rec->data_size == 0)) {
        ev_io_stop(loop, watcher);
        senders_map.erase(watcher->fd);
        free(watcher);
        if (mes_len == -1 || rec->closed) {
            reciever_destroy(rec);
            sender_reciever[fd] = NULL;
            sender_reciever.erase(fd);
        }
    }
}

void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    reciever *rec = sender_reciever[watcher->fd];
    if (rec == NULL) {
        ev_io_stop(loop, watcher);
        free(watcher);
        sender_reciever.erase(watcher->fd);
        return;
    }
    int was_empty = 0;
    if (rec->data_size != 0) {
        was_empty = 1;
    }
    if (rec->buffer_size == rec->data_size)
        return;

    int mes_len = reciever_read(rec, BUFFER_SIZE);
    if (mes_len <= 0) {
        ev_io_stop(loop, watcher);
        free(watcher);
        rec->closed = 1;
        if (rec->data_size == 0) {
            reciever_destroy(rec);
            sender_reciever[watcher->fd] = NULL;
            sender_reciever.erase(watcher->fd);
        }
        return;
    }
    int reciever_id = rec->reciever_fd;
    senders_map[reciever_id] = watcher->fd;
    if (was_empty) {
        struct ev_io *write_watcher = (struct ev_io *) calloc(1, sizeof(*write_watcher));
        ev_io_init(write_watcher, write_cb, reciever_id, EV_WRITE);
        ev_io_start(loop, write_watcher);
    }
}

void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    std::cout << "Accept \n";

    int rand_number = rand() % config_map[watcher->fd].size();
    char* ip = config_map[watcher->fd][rand_number].first;
    int port = config_map[watcher->fd][rand_number].second;

    int client_socket = create_client_socket(watcher->fd);
    if (is_invalid(client_socket)) {
        return;
    }

    int server_socket = create_server_socket(ip, port);
    if (is_invalid(server_socket)) {
        shutdown(client_socket, SHUT_RDWR);
        close(client_socket);
        return;
    }

    reciever *rec_serv, *rec_client;

    rec_client = reciever_init(BUFFER_SIZE, client_socket, server_socket);
    rec_serv = reciever_init(BUFFER_SIZE, server_socket, client_socket);
    sender_reciever[client_socket] = rec_serv;
    sender_reciever[server_socket] = rec_client;

    struct ev_io *client_watcher = (struct ev_io *) calloc(1, sizeof(*client_watcher));
    ev_io_init(client_watcher, read_cb, client_socket, EV_READ);
    ev_io_start(loop, client_watcher);

    struct ev_io *server_watcher = (struct ev_io *) calloc(1, sizeof(*server_watcher));
    ev_io_init(server_watcher, read_cb, server_socket, EV_READ);
    ev_io_start(loop, server_watcher);

}

int main(int argc, char ** argv)
{
    if (argc < 2) {
        std::cout << "No name of config file\n";
        return -1;
    }
    std::string config = argv[1];

    struct ev_loop *loop = ev_default_loop(0);
    ev_io accept_watchers[COUNT_PORTS];
    int corrent_watcher = 0;
    std::vector<int> master_sockets = read_config(config.c_str(), config_map);
    auto it = master_sockets.begin();
    for (;it != master_sockets.end(); it++) {
        ev_io_init(&accept_watchers[corrent_watcher], accept_cb, *it, EV_READ);
        ev_io_start(loop, &accept_watchers[corrent_watcher]);
        corrent_watcher++;
    }
    ev_run(loop);
    return 0;
}
