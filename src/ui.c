#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>


static char *prompt_user_generic(const char *msg, char *buf, size_t n) {
    printf("\033[2J\033[H");
    printf("%s", msg);
    fflush(stdout);

    if (fgets(buf, n, stdin)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    } else {
        buf[0] = '\0';
    }

    return buf;
}


char *prompt_user_name_1(char *buf, size_t n) {
    return prompt_user_generic("enter your terminal nickname: ", buf, n);
}

char *prompt_user_name_2(char *buf, size_t n) {
    return prompt_user_generic("somebody wants to talk to you!\nenter your terminal nickname: ", buf, n);
}