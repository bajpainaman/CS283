#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 * Helper to trim leading/trailing whitespace in-place
 */
static char *trim_spaces(char *str)
{
    while (isspace((unsigned char)*str)) {
        str++;
    }
    if (*str == '\0') {
        return str;
    }
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    end[1] = '\0';
    return str;
}

/*
 * build_cmd_list
 *   Splits cmd_line by '|', parses each segment into (exe, args).
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    // 1) Trim overall spaces
    char *line = trim_spaces(cmd_line);

    // 2) Check if empty => no commands
    if (*line == '\0') {
        return WARN_NO_CMDS;
    }

    // 3) Split by '|'
    int count = 0;
    char *saveptr;
    char *segment = strtok_r(line, "|", &saveptr);

    while (segment != NULL) {
        segment = trim_spaces(segment);
        if (*segment != '\0') {
            if (count >= CMD_MAX) {
                return ERR_TOO_MANY_COMMANDS;
            }
            // Parse the segment
            char *saveptr2;
            char *token = strtok_r(segment, " \t\r\n", &saveptr2);
            if (!token) {
                segment = strtok_r(NULL, "|", &saveptr);
                continue;
            }
            // first token => exe
            if (strlen(token) >= EXE_MAX) {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            strcpy(clist->commands[count].exe, token);

            // gather remaining tokens as args
            char temp_args[ARG_MAX];
            temp_args[0] = '\0';

            token = strtok_r(NULL, " \t\r\n", &saveptr2);
            while (token != NULL) {
                if (strlen(temp_args) + strlen(token) + 2 >= ARG_MAX) {
                    return ERR_CMD_OR_ARGS_TOO_BIG;
                }
                if (temp_args[0] != '\0') {
                    strcat(temp_args, " ");
                }
                strcat(temp_args, token);
                token = strtok_r(NULL, " \t\r\n", &saveptr2);
            }

            strcpy(clist->commands[count].args, temp_args);
            count++;
        }
        segment = strtok_r(NULL, "|", &saveptr);
    }

    if (count == 0) {
        return WARN_NO_CMDS;
    }
    clist->num = count;
    return OK;
}

/* ------------------------------------------------------------------
 * EXTRA CREDIT: DRAGON in ASCII, stored in Base64, decoded on the fly
 *
 * The ASCII from the assignment is below. We place it in a single
 * base64-encoded string. If you want to see how we got this, we put
 * that ASCII text into "dragon.txt" and ran:
 *    base64 -w 0 dragon.txt > dragon.b64
 */

/*
   This is the FULL ASCII dragon from the assignment, base64-encoded.
   DO NOT break or cut it; keep the entire line so decoding is correct.
*/
static const char DRAGON_B64[] =
"ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgQCUlJSUgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgJSUlJSUlICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlICUlJSUlJSUgICAgICAgICAgIEAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSAgICAgICAgJSUlJSUlJSAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUgICUlJSVAICAgICAgICAgJSUlJSUlJSUlJSUlQCAgICAlJSUlJSUgIEAlJSUlICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgJSUlJSUlJSUlJSUlICUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAlJSUgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlQCBAJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAlJSAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlQCUlJSUlJUAgICAgICAgICAgICAgIAogICAgICAlJSUlJSUlJUAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgJSUgICAgICAgICAgICAgICAgCiAgICAlJSUlJSUlJSUlJSUlICAgICAgICAgJSVAJSUlJSUlJSUlJSUlICAgICAgICAgICAlJSUlJSUlJSUlJSAlJSUlJSUlJSUlJSUgICAgICBAJSAgICAgICAgICAgICAgICAKICAlJSUlJSUlJSUlICAgJSUlICAgICAgICAlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgICAgICAgICAgIAogJSUlJSUlJSUlICAgICAgICUgICAgICAgICAlJSUlJSUlJSUlJSUlICAgICAgICAgICAgICUlJSUlJSUlJSUlJUAlJSUlJSUlJSUlJSAgICAgICAgICAgICAgICAgICAgICAgCiUlJSUlJSUlJUAgICAgICAgICAgICAgICAgJSAlJSUlJSUlJSUlJSUlICAgICAgICAgICAgQCUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgICAgICAgICAgICAKJSUlJSUlJSVAICAgICAgICAgICAgICAgICAlJUAlJSUlJSUlJSUlJSUgICAgICAgICAgICBAJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgICAgIAolJSUlJSUlQCAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgCiUlJSUlJSUlJSUgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAlJSUlICAKJSUlJSUlJSUlQCAgICAgICAgICAgICAgICAgICBAJSUlJSUlJSUlJSUlJSUgICAgICAgICAlJSUlJSUlJSUlJSVAICUlJSUgJSUlJSUlJSUlJSUlJSUlJSUgICAlJSUlJSUlJQolJSUlJSUlJSUlICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUgICAgICAgICUlJSUlJSUlJSUlJSUgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUgJSUlJSUlJSUlCiUlJSUlJSUlJUAlJUAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJUAgICAgICAgJSUlJSUlJSUlJSUlJSUgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgJSUKICUlJSUlJSUlJSUgICAgICAgICAgICAgICAgICAlICUlJSUlJSUlJSUlJSUlQCAgICAgICAgJSUlJSUlJSUlJSUlJSUgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAlJQogICUlJSUlJSUlJSUlJSAgQCAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAlJSUgCiAgICUlJSUlJSUlJSUlJSUgJSUgICUgICVAICUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICUlJSAKICAgICUlJSUlJSUlJSUlJSUlJSUlJSAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAgICBAJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAlJSUlJSUlIAogICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgJSUlICAgCiAgICAgIEAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgICAgICAKICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlICAlJSUlJSUlICAgICAgICAgIAogICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAgICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlICBAJSUlJSUlJSUlICAgICAgICAgCiAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgIEAlQCUgICAgICAgICAgICAgICAgICBAJSUlJSUlJSUlJSUlJSUlJSUlICAgJSUlICAgICAgICAKICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlICAgICAgICAlJSUlJSUlJSUlICAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUgICAgJSAgICAgICAgIAogICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgJSUlJSAlJSUgICAgICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSAgJSUlQCAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSAlJSUlJSUgJSUgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSVAICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlQCAgICAgICAK";

/* 
 * A minimal base64 decoder. For large data, you'd typically use a library,
 * but this is enough for demonstration. 
 */
static unsigned char *decode_base64(const char *in, size_t *out_size)
{
    static unsigned char B64_INDEX[256];
    static int init_table = 0;

    if (!init_table) {
        // fill everything as invalid
        for (int i = 0; i < 256; i++) {
            B64_INDEX[i] = 0xFF;
        }
        // standard alphabet
        const char *alphabet =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (int i = 0; i < 64; i++) {
            B64_INDEX[(unsigned char)alphabet[i]] = (unsigned char)i;
        }
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
        unsigned char c = (unsigned char)in[i];
        if (c == '=') {
            // padding
            break;
        }
        unsigned char d = B64_INDEX[c];
        if (d == 0xFF) {
            // ignore invalid chars
            continue;
        }
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

void print_dragon(void)
{
    size_t decoded_size = 0;
    unsigned char *decoded = decode_base64(DRAGON_B64, &decoded_size);
    if (decoded) {
        // Print entire decoded ASCII
        printf("%s\n", decoded);
        free(decoded);
    } else {
        printf("[Error decoding DRAGON]\n");
    }
}