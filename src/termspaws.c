#define _GNU_SOURCE
#include "termspawn.h"
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

extern char **environ;

int spawn_chat_terminal(const char *self_exe, const char *sock_path, const char *msg, pid_t *out_pid) {

    pid_t pid;
    int rc = 0;

    // try gnome-terminal
    const char *terms[] = {"gnome-terminal", "x-terminal-emulator", "xterm"};
    for (int i = 0; i < 3; i++) {
        const char *term = terms[i];
        char *argv[7];
        int j = 0;
        argv[j++] = (char*)term;
        argv[j++] = (char*)"--";
        argv[j++] = (char*)self_exe;
        argv[j++] = (char*)"--chat";
        argv[j++] = (char*)sock_path;

        if(msg) argv[j++] = (char*)msg;

        argv[j] = NULL;
        rc = posix_spawnp(&pid, term, NULL, NULL, argv, environ);
        if (rc == 0) break; // success
    }

    if (rc != 0) {
        perror("posix_spawnp");
        return 0;
    }

    if (out_pid) *out_pid = pid;
    return 1;
}
