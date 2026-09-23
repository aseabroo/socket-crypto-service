#include "common.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

int parse_port(const char *text, unsigned short *port_out) {
    if (!text || !*text || !port_out) {
        return -1;
    }

    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);

    if (errno != 0 || *end != '\0' || value < 1 || value > 65535) {
        return -1;
    }

    *port_out = (unsigned short)value;
    return 0;
}

int validate_text(const char *text) {
    if (!text) {
        return 0;
    }

    for (const unsigned char *p = (const unsigned char *)text; *p; ++p) {
        if ((*p < 'A' || *p > 'Z') && *p != ' ') {
            return 0;
        }
    }

    return 1;
}

static int char_to_value(char c) {
    return c == ' ' ? 26 : c - 'A';
}

static char value_to_char(int value) {
    return value == 26 ? ' ' : (char)('A' + value);
}

int transform_text(const char *text, const char *key, int decrypt, char *out) {
    if (!text || !key || !out) {
        return -1;
    }

    size_t text_len = strlen(text);
    size_t key_len = strlen(key);

    if (key_len < text_len || !validate_text(text) || !validate_text(key)) {
        return -1;
    }

    for (size_t i = 0; i < text_len; ++i) {
        int t = char_to_value(text[i]);
        int k = char_to_value(key[i]);
        int value = decrypt ? (t - k + 27) % 27 : (t + k) % 27;
        out[i] = value_to_char(value);
    }

    out[text_len] = '\0';
    return 0;
}
