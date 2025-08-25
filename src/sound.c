#include "sound.h"
#include <stdio.h>
#include <stdlib.h>

const char *NEW_MESSAGE = "sounds/new_message.mp3";

void play_sound(const char *path) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mpg123 %s > /dev/null 2>&1 &", path);
    system(cmd);
}