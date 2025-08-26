#include "sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>  
#include <signal.h>
#include <spawn.h>
#include <fcntl.h>

extern char **environ;

const char *NEW_MESSAGE = "sounds/new_message.mp3";

// non-blocking sound player
void play_sound(const char *path) {

    if (!path) return;

    pid_t pid;
    char *argv[] = {"/usr/bin/mpg123", (char*)path, NULL};

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);

    // redirect stdout / stderr to /dev/null
    posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0);
    posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0);

    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP);

    int rc = posix_spawn(&pid, "/usr/bin/mpg123", &actions, &attr, argv, environ);
    posix_spawn_file_actions_destroy(&actions);
    posix_spawnattr_destroy(&attr);

    if (rc != 0) {
        perror("posix_spawn");
        return;
    }
}