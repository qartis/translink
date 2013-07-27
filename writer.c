#define _XOPEN_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "translink.h"

char *urldecode(const char *input){
    char *nl = strchr(input, '\n');
    if (nl){
        *nl = '\0';
    }
    char *buf = malloc(strlen(input));
    char *cur = buf;
    while (*input){
        if (input[0] == '%' &&
                isxdigit(input[1]) &&
                isxdigit(input[2])){
            int digit;
            sscanf(input, "%%%02x", &digit);
            *cur = (char)digit;
            cur++;
            input += 3;
        } else {
            *cur = *input;
            cur++;
            input++;
        }
    }
    *cur = '\0';
    return buf;
}

int main(int argc, char **argv){
    struct tm tm = {0};
    int n_zones, z_issued;
    char *str = getenv("QUERY_STRING");

    if (str){
        str = urldecode(str);
    } else if (argc > 1){
        printf("str: '%s\n", argv[1]);
        str = argv[1];
    } else {
        printf("usage: %s \"Y/M/D 06:18 1 2\"\nfor a 2z ticket from zone -1-\n", argv[0]);
        exit(1);
    }

    strptime(str, "%Y/%m/%d %H:%M", &tm);

    char *space = strchr(str, ' ');
    space = strchr(space + 1, ' ');
    sscanf(space, "%d %d", &z_issued, &n_zones);
    char *bits = encode(tm, z_issued, n_zones);
    printf("bits: %s\n", bits);
    uint8_t bytes[64];
    int len = reformat(bits, bytes);
    int i;
    for (i=0;i<len;i++){
        printf("%02x ", bytes[i]);
    }
    printf("\n");
}
