#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dshlib.h"

/*
 * decode_base64:
 *   Minimal Base64 decoder to decode the Drexel Dragon art.
 */
static unsigned char *decode_base64(const char *in, size_t *out_size) {
    static unsigned char B64_INDEX[256];
    static int init_table = 0;
    
    if (!init_table) {
        for (int i = 0; i < 256; i++)
            B64_INDEX[i] = 0xFF;
        const char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (int i = 0; i < 64; i++)
            B64_INDEX[(unsigned char)alphabet[i]] = (unsigned char)i;
        init_table = 1;
    }
    
    size_t in_len = strlen(in);
    size_t max_decoded = (in_len / 4) * 3 + 4;
    unsigned char *out = (unsigned char*)malloc(max_decoded);
    if (!out) {
        *out_size = 0;
        return NULL;
    }
    
    int val = 0, valb = -8;
    size_t idx = 0;
    for (size_t i = 0; i < in_len; i++) {
        unsigned char c = in[i];
        if (c == '=')
            break;
        unsigned char d = B64_INDEX[c];
        if (d == 0xFF)
            continue;
        val = (val << 6) + d;
        valb += 6;
        if (valb >= 0) {
            out[idx++] = (unsigned char)((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    out[idx] = '\0';
    *out_size = idx;
    return out;
}

/*
 * print_dragon:
 *   Decodes and prints the Base64-encoded Drexel Dragon ASCII art.
 */
void print_dragon(void) {
    static const char DRAGON_B64[] =
"ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgQCUlJSUgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlICUlJSUlJSUgICAgICAgICAgIEAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSAgICAgICAgJSUlJSUlJSAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUgICUlJSVAICAgICAgICAgJSUlJSUlJSUlJSUlQCAgICAlJSUlJSUgIEAlJSUlICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgJSUlJSUlJSUlJSUlICUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAlJSUgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlQCBAJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAlJSAgICAgICAgICAgIAogICAgICAlJSUlJSUlJUAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgJSUgICAgICAgICAgICAgICAgCiAgICAlJSUlJSUlJSUlJSUlICAgICAgICAgJSVAJSUlJSUlJSUlJSUlICAgICAgICAgICAlJSUlJSUlJSUlJSAlJSUlJSUlJSUlJSUgICAgICBAJSAgICAgICAgICAgICAgICAKICAlJSUlJSUlJSUlICAgJSUlICAgICAgICAlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgICAlJSUlJSUlJSUlJUAlJSUlJSUlJSUlJSAgICAgICAgICAgICAgICAgICAgICAgCiUlJSUlJSUlJUAgICAgICAgICAgICAgICAgJSAlJSUlJSUlJSUlJSUlICAgICAgICAgICAgQCUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgICAgICAgICAgICAKJSUlJSUlJSVAICAgICAgICAgICAgICAgICAlJUAlJSUlJSUlJSUlJSUgICAgICAgICAgICBAJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgICAgIAolJSUlJSUlQCAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgCiUlJSUlJSUlJSUgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAlJSUlICAKJSUlJSUlJSUlQCAgICAgICAgICAgICAgICAgICBAJSUlJSUlJSUlJSUlJSUgICAgICAgICAlJSUlJSUlJSUlJSUlJSUgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAlJQogICUlJSUlJSUlJSUlJSAgQCAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAlJSUgCiAgICUlJSUlJSUlJSUlJSUgJSUgICUgICVAICUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICUlJSAKICAgICUlJSUlJSUlJSUlJSUlJSUlJSAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAgICBAJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAlJSUlJSUlIA==";
    
    size_t out_size = 0;
    unsigned char *decoded = decode_base64(DRAGON_B64, &out_size);
    if (decoded) {
        printf("%s\n", decoded);
        free(decoded);
    } else {
        fprintf(stderr, "Error decoding dragon art\n");
    }
}

int main(void) {
    print_dragon();
    return 0;
}
