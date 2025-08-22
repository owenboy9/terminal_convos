#include "ui.h"
#include <stdio.h>

void prompt_user(int *work, int *breaktime, int *rounds) {
    printf("\033[2J\033[H"); // clear screen: \033[2J = ANSI escape code to clear the screen; \033[H Moves cursor to the home position (top-left)
    printf("okay, time to start!\n");

    printf("enter work session duration in minutes: ");
    fflush(stdout);
    scanf("%d", work);

    printf("enter break duration in minutes: ");
    fflush(stdout);
    scanf("%d", breaktime);

    printf("enter number of rounds: ");
    fflush(stdout);
    scanf("%d", rounds);
}