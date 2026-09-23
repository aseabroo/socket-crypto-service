#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "protocol.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int connect_to(const char *host, unsigned short port) {
    char port_text[16];
    snprintf(port_text, sizeof(port_text), "%u", port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *addresses = NULL;
    if (getaddrinfo(host, port_text, &hints, &addresses) != 0) {
        return -1;
    }

    int fd = -1;
    for (struct addrinfo *it = addresses; it; it = it->ai_next) {
        fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (fd < 0) {
            continue;
        }

        if (connect(fd, it->ai_addr, it->ai_addrlen) == 0) {
            break;
        }

        close(fd);
        fd = -1;
    }

    freeaddrinfo(addresses);
    return fd;
}

int main(int argc, char **argv) {
    if (argc != 6) {
        fprintf(stderr, "usage: %s enc|dec HOST PORT TEXT KEY\n", argv[0]);
        return 1;
    }

    uint8_t operation = 0;
    if (strcmp(argv[1], "enc") == 0) {
        operation = OP_ENCRYPT;
    } else if (strcmp(argv[1], "dec") == 0) {
        operation = OP_DECRYPT;
    } else {
        fprintf(stderr, "operation must be enc or dec\n");
        return 1;
    }

    unsigned short port = 0;
    if (parse_port(argv[3], &port) < 0) {
        fprintf(stderr, "invalid port: %s\n", argv[3]);
        return 1;
    }

    const char *text = argv[4];
    const char *key = argv[5];

    if (!validate_text(text) || !validate_text(key)) {
        fprintf(stderr, "text and key may contain only A-Z and space\n");
        return 1;
    }

    if (strlen(key) < strlen(text)) {
        fprintf(stderr, "key must be at least as long as text\n");
        return 1;
    }

    int fd = connect_to(argv[2], port);
    if (fd < 0) {
        fprintf(stderr, "could not connect to %s:%u\n", argv[2], port);
        return 2;
    }

    if (send_request(fd, operation, text, key) < 0) {
        fprintf(stderr, "failed to send request\n");
        close(fd);
        return 2;
    }

    char *response = NULL;
    if (recv_response(fd, &response) < 0) {
        fprintf(stderr, "failed to receive response\n");
        close(fd);
        return 2;
    }

    printf("%s\n", response);

    free(response);
    close(fd);
    return 0;
}
