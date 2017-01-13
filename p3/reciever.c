#include "reciever.h"

reciever* reciever_init(int size_param, int recv_fd, int send_fd) {
    reciever *recv = (reciever*) calloc(1, sizeof(*recv));
    recv->buf = (char*) calloc(size_param, sizeof(char));
    recv->tmp_buf = (char*) calloc(size_param, sizeof(char));
    recv->begin = 0;
    recv->buffer_size = size_param;
    recv->data_size = 0;
    recv->reciever_fd = recv_fd;
    recv->sender_fd = send_fd;
    recv->closed = 0;
    return recv;
}

void reciever_destroy(reciever *recv) {
    free(recv->buf);
    free(recv->tmp_buf);
    free(recv);
}

int reciever_read(reciever *rcv, int size) {
    int free_size = rcv->buffer_size - rcv->data_size;
    if (free_size == 0) {
        return 0;
    }
    int cur_size = size;
    if (free_size < cur_size) {
        cur_size = free_size;
    }

    int mes_len = recv(rcv->sender_fd, rcv->tmp_buf, cur_size, MSG_NOSIGNAL); // length of message
    if (mes_len <= 0) {
        return mes_len;
    }
    int begin_free_data = (rcv->begin + rcv->data_size) % rcv->buffer_size;
    free_size = rcv->buffer_size - begin_free_data;
    if (free_size < mes_len) {
        memcpy(&rcv->buf[0], &rcv->tmp_buf[free_size], mes_len - free_size);
    } else {
        memcpy(&rcv->buf[begin_free_data], &rcv->tmp_buf[0], mes_len);
    }

    rcv->data_size += mes_len;
    return mes_len;
}


int reciever_write(reciever *rcv, int size) {
    int cur_size = size;
    if (rcv->data_size < cur_size) {
        cur_size = rcv->data_size;
    }
    if (cur_size == 0) {
        return 0;
    }
    memcpy(&rcv->tmp_buf[0], &rcv->buf[rcv->begin], cur_size);
    int mes_len = send(rcv->reciever_fd, rcv->tmp_buf, cur_size, MSG_NOSIGNAL);
    if (mes_len <= 0)
        return mes_len;
    rcv->begin = (rcv->begin + mes_len) % rcv->buffer_size;
    rcv->data_size -= mes_len;
    return mes_len;
}
