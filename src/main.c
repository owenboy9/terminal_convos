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
    // timer mode?
    if (argc >= 2 && strcmp(argv[1], "--timer") == 0) {
        if (argc != 6){
            fprintf(stderr, "usage: %s --timer <sock> <work> <break> <rounds>\n", argv[0]);
            return 1;
        }
        const char *sock = argv[2];
        int work = atoi(argv[3]);
        int brk = atoi(argv[4]);
        int rounds = atoi(argv[5]);
        return run_timer(sock, work, brk, rounds);
    }

    // controller mode
    int work, brk, rounds;
    prompt_user(&work, &brk, &rounds);

    char exe[PATH_MAX];
    self_exe_path(exe, sizeof(exe));

    return run_controller(exe, work, brk, rounds);
}