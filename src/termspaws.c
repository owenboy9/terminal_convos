#define _GNU_SOURCE
#include "termspawn.h"
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

extern char **environ;

int spawn_timer_terminal(const char *self_exe, const char *sock_path, int work_min, int break_min, int rounds, pid_t *out_pid) {

    char w[16], b[16], r[16];
    snprintf(w, sizeof(w), "%d", work_min);
    snprintf(b, sizeof(b), "%d", break_min);
    snprintf(r, sizeof(r), "%d", rounds);

    pid_t pid;
    int rc = 0;

    // try gnome-terminal
    const char *term = "gnome-terminal";
    char *argv[] = {
        (char*)term, (char*)"--", (char*)self_exe, (char*)"--timer",
        (char*)sock_path, w, b, r, NULL
    };
    rc = posix_spawnp(&pid, term, NULL, NULL, argv, environ);

    if (rc != 0) {
        // fallback to ubuntu standard terminal
        term = "x-terminal-emulator";
        char *argv2[] = {
            (char*)term, (char*)"-e", (char*)self_exe, (char*)"--timer",
            (char*)sock_path, w, b, r, NULL
        };
        rc = posix_spawnp(&pid, term, NULL, NULL, argv2, environ);

        if (rc != 0) {
            // fallback to xterm
            term = "xterm";
            char *argv3[] = {
                (char*)term, (char*)"-e", (char*)self_exe, (char*)"--timer",
                (char*)sock_path, w, b, r, NULL
            };
            rc = posix_spawnp(&pid, term, NULL, NULL, argv3, environ);
        }
    }

    if (rc != 0) {
        perror("posix_spawnp");
        return 0;
    }

    if (out_pid) *out_pid = pid;
    return 1;
}
