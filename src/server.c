#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "protocol.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static int create_listener(unsigned short port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }

    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0 ||
        listen(fd, 16) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static void handle_client(int fd) {
    uint8_t operation = 0;
    char *text = NULL;
    char *key = NULL;

    if (recv_request(fd, &operation, &text, &key) < 0) {
        close(fd);
        _exit(1);
    }

    if ((operation != OP_ENCRYPT && operation != OP_DECRYPT) ||
        !validate_text(text) || !validate_text(key)) {
        free(text);
        free(key);
        close(fd);
        _exit(1);
    }

    size_t length = strlen(text);
    char *result = calloc(length + 1, 1);
    if (!result) {
        free(text);
        free(key);
        close(fd);
        _exit(1);
    }

    if (transform_text(text, key, operation == OP_DECRYPT, result) == 0) {
        (void)send_response(fd, result);
    }

    free(result);
    free(text);
    free(key);
    close(fd);
    _exit(0);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s PORT\n", argv[0]);
        return 1;
    }

    unsigned short port = 0;
    if (parse_port(argv[1], &port) < 0) {
        fprintf(stderr, "invalid port: %s\n", argv[1]);
        return 1;
    }

    struct sigaction child_action = {0};
    child_action.sa_handler = SIG_IGN;
    child_action.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&child_action.sa_mask);
    sigaction(SIGCHLD, &child_action, NULL);

    int listener = create_listener(port);
    if (listener < 0) {
        perror("crypto-server");
        return 1;
    }

    printf("crypto-server listening on port %u\n", port);
    fflush(stdout);

    for (;;) {
        int client = accept(listener, NULL, NULL);
        if (client < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            continue;
        }

        pid_t child = fork();
        if (child < 0) {
            perror("fork");
            close(client);
            continue;
        }

        if (child == 0) {
            close(listener);
            handle_client(client);
        }

        close(client);
    }
}
