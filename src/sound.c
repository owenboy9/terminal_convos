#define _GNU_SOURCE
#include "sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <linux/limits.h>
#include <unistd.h>

const char *NEW_MESSAGE = "sounds/new_message.mp3";

void play_sound(const char *file) {
    if (!file) return;

    char exe[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe, sizeof(exe)-1);
    if (len < 0) return;
    exe[len] = '\0';

    char *dir = dirname(exe);

    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/../%s", dir, file);

    char cmd[PATH_MAX + 64]; // extra space for command
    snprintf(cmd, sizeof(cmd), "mpg123 %s > /dev/null 2>&1 &", path);
    system(cmd);
}