#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

static int fill_random(unsigned char *buffer, size_t length) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    size_t total = 0;
    while (total < length) {
        ssize_t received = read(fd, buffer + total, length - total);
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(fd);
            return -1;
        }
        if (received == 0) {
            close(fd);
            return -1;
        }
        total += (size_t)received;
    }

    close(fd);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s LENGTH\n", argv[0]);
        return 1;
    }

    char *end = NULL;
    errno = 0;
    unsigned long length = strtoul(argv[1], &end, 10);

    if (errno != 0 || *end != '\0' || length == 0 || length > 1048576UL) {
        fprintf(stderr, "length must be between 1 and 1048576\n");
        return 1;
    }

    unsigned char *random_bytes = malloc(length);
    if (!random_bytes) {
        perror("malloc");
        return 1;
    }

    if (fill_random(random_bytes, length) < 0) {
        perror("random");
        free(random_bytes);
        return 1;
    }

    size_t alphabet_size = sizeof(alphabet) - 1;
    for (unsigned long i = 0; i < length; ++i) {
        putchar(alphabet[random_bytes[i] % alphabet_size]);
    }
    putchar('\n');

    free(random_bytes);
    return 0;
}
