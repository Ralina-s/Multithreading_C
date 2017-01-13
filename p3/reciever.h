#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>

struct reciever_struct
{
    char *buf, *tmp_buf;
    int buffer_size,  data_size;
    int begin;
    int reciever_fd,  sender_fd;
    int closed;
};

typedef struct reciever_struct reciever;

reciever* reciever_init(int size_param, int recv_fd, int send_fd);

void reciever_destroy(reciever *recv);

int reciever_read(reciever *recv, int size);

int reciever_write(reciever *recv, int size);
