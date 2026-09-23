#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

#define MAX_PAYLOAD 1048576U

int parse_port(const char *text, unsigned short *port_out);
int validate_text(const char *text);
int transform_text(const char *text, const char *key, int decrypt, char *out);

#endif
