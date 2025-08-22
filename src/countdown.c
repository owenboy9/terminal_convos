#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void countdown(int minutes, const char *label) {
    int seconds = minutes * 60;
    if (minutes == 1) {
        printf("%s for one minute...\n", label);
    } else {
        printf("%s for %d minutes...\n", label, minutes);
    }

    while (seconds >= 0) {
        int mins = seconds / 60;
        int secs = seconds % 60;
        printf("\r%02d:%02d remaining", mins, secs);
        fflush(stdout);
        if (seconds-- == 0) break;
        sleep(1);
    }
    printf("\r%s done!          \n", label);
}