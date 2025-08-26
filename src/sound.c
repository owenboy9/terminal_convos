#define _GNU_SOURCE
#include "sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <linux/limits.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>

const char *NEW_MESSAGE = "sounds/new_message.mp3";

// SIGCHLD handler to reap backgrounded mpg123 processes
static void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// install SIGCHLD handler once
__attribute__((constructor))
static void install_sigchld_handler(void) {
    struct sigaction sa = {0};
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
}

void play_sound(const char *file) {
    if (!file) return;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        setsid(); // detach from terminal

        // redirect stdout & stderr to /dev/null
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        // reslove path relative to executable
        char exe[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", exe, sizeof(exe)-1);
        if (len < 0) return;
        exe[len] = '\0';

        char *dir = dirname(exe);

        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/../%s", dir, file);

        char *argv[] = {"mpg123", path, NULL};
        execvp("mpg123", argv);

        // execvp failed
        perror("execvp");
        _exit(1);
    }
}