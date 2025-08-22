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

    char msg_buf[256];
    snprintf(msg_buf, sizeof(msg_buf), "%s", msg);

    pid_t pid;
    int rc = 0;

    // try gnome-terminal
    const char *terms[] = {"gnome-terminal", "x-terminal-emulator", "xterm"};
    for (int i = 0; i < 3; i++) {
        const char *term = terms[i];
        char *argv[] = { (char*)term, (char*)"--", (char*)self_exe, (char*)"--chat", (char*)sock_path, msg_buf, NULL };
        rc = posix_spawn(&pid, term, NULL, NULL, argv, environ);
        if (rc == 0) break; // success
    }

    if (rc != 0) {
        perror("posix_spawnp");
        return 0;
    }

    if (out_pid) *out_pid = pid;
    return 1;
}
