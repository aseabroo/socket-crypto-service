#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define OP_ENCRYPT 1
#define OP_DECRYPT 2

int send_all(int fd, const void *buffer, size_t length);
int recv_all(int fd, void *buffer, size_t length);

int send_request(int fd, uint8_t operation, const char *text, const char *key);
int recv_request(int fd, uint8_t *operation, char **text_out, char **key_out);

int send_response(int fd, const char *text);
int recv_response(int fd, char **text_out);

#endif
