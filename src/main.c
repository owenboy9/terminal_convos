#define _GNU_SOURCE
#include "ui.h"
#include "roles.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>

static void self_exe_path(char *out, size_t n) {
    ssize_t m = readlink("/proc/self/exe", out, n-1);
    if (m >=0) {
        out[m] = '\0';
        return;
    }
    snprintf(out, n, "./app");
}

int main(int argc, char **argv) {
    // chat mode?
    if (argc >= 2 && strcmp(argv[1], "--chat") == 0) {
        if (argc != 3){
            fprintf(stderr, "usage: %s --chat <sock>\n", argv[0]);
            return 1;
        }
        const char *sock = argv[2];
        return run_chat(sock);
    }

    char name[56];
    // controller mode
    prompt_user_name1(name);

    char exe[PATH_MAX];
    self_exe_path(exe, sizeof(exe));

    return run_controller(exe);
}