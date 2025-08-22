#include "sound.h"
#include <stdio.h>
#include <stdlib.h>

const char *START_WORK_SOUND = "sounds/start_work.mp3";
const char *END_DAY_SOUND = "sounds/end_day.mp3";
const char *START_BREAK_SOUND = "sounds/start_break.mp3";

void play_sound(const char *path) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mpg123 %s > /dev/null 2>&1", path);
    system(cmd);
}