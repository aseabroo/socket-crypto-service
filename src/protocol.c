#include "protocol.h"
#include "common.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

int send_all(int fd, const void *buffer, size_t length) {
    const unsigned char *cursor = buffer;
    size_t sent_total = 0;

    while (sent_total < length) {
        ssize_t sent = send(fd, cursor + sent_total, length - sent_total, 0);
        if (sent < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (sent == 0) {
            return -1;
        }
        sent_total += (size_t)sent;
    }

    return 0;
}

int recv_all(int fd, void *buffer, size_t length) {
    unsigned char *cursor = buffer;
    size_t received_total = 0;

    while (received_total < length) {
        ssize_t received = recv(fd, cursor + received_total, length - received_total, 0);
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (received == 0) {
            return -1;
        }
        received_total += (size_t)received;
    }

    return 0;
}

int send_request(int fd, uint8_t operation, const char *text, const char *key) {
    size_t text_len = strlen(text);
    size_t key_len = strlen(key);

    if (text_len > MAX_PAYLOAD || key_len > MAX_PAYLOAD) {
        return -1;
    }

    uint32_t text_net = htonl((uint32_t)text_len);
    uint32_t key_net = htonl((uint32_t)key_len);

    if (send_all(fd, &operation, sizeof(operation)) < 0 ||
        send_all(fd, &text_net, sizeof(text_net)) < 0 ||
        send_all(fd, &key_net, sizeof(key_net)) < 0 ||
        send_all(fd, text, text_len) < 0 ||
        send_all(fd, key, key_len) < 0) {
        return -1;
    }

    return 0;
}

int recv_request(int fd, uint8_t *operation, char **text_out, char **key_out) {
    uint32_t text_net = 0;
    uint32_t key_net = 0;

    if (recv_all(fd, operation, sizeof(*operation)) < 0 ||
        recv_all(fd, &text_net, sizeof(text_net)) < 0 ||
        recv_all(fd, &key_net, sizeof(key_net)) < 0) {
        return -1;
    }

    uint32_t text_len = ntohl(text_net);
    uint32_t key_len = ntohl(key_net);

    if (text_len > MAX_PAYLOAD || key_len > MAX_PAYLOAD || key_len < text_len) {
        return -1;
    }

    char *text = calloc((size_t)text_len + 1, 1);
    char *key = calloc((size_t)key_len + 1, 1);

    if (!text || !key) {
        free(text);
        free(key);
        return -1;
    }

    if (recv_all(fd, text, text_len) < 0 || recv_all(fd, key, key_len) < 0) {
        free(text);
        free(key);
        return -1;
    }

    *text_out = text;
    *key_out = key;
    return 0;
}

int send_response(int fd, const char *text) {
    size_t length = strlen(text);
    if (length > MAX_PAYLOAD) {
        return -1;
    }

    uint32_t net_length = htonl((uint32_t)length);
    if (send_all(fd, &net_length, sizeof(net_length)) < 0 ||
        send_all(fd, text, length) < 0) {
        return -1;
    }

    return 0;
}

int recv_response(int fd, char **text_out) {
    uint32_t net_length = 0;

    if (recv_all(fd, &net_length, sizeof(net_length)) < 0) {
        return -1;
    }

    uint32_t length = ntohl(net_length);
    if (length > MAX_PAYLOAD) {
        return -1;
    }

    char *text = calloc((size_t)length + 1, 1);
    if (!text) {
        return -1;
    }

    if (recv_all(fd, text, length) < 0) {
        free(text);
        return -1;
    }

    *text_out = text;
    return 0;
}
