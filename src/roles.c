#include "roles.h"
#include "ipc.h"
#include "termspawn.h"
#include "countdown.h"
#include "sound.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>


// cleanup helper
static void cleanup_server(IpcEndpoint *srv, pid_t timer_pid) {
    if (timer_pid > 0) {
        int status;
        waitpid(timer_pid, &status, 0);
    }
    ipc_server_cleanup(srv);
}

// check expected msg
static int recv_expect(IpcEndpoint *ep, const char *expected) {
    char buf[256];
    if (!ipc_recvline(ep->conn_fd, buf, sizeof(buf))) return 0;
    if (strcmp(buf, expected) != 0) {
        fprintf(stderr, "unexpected message: %s (expected: %s)\n", buf, expected);
        return 0;
    }
    return 1;
}

// controller: creates server, spawns terminal for timer, accepts conn, orchestrates work-break flow

int run_controller(const char *self_exe, int work, int brk, int rounds) {
    IpcEndpoint srv = {0};
    pid_t timer_pid = 0;

    if (!ipc_server_start(&srv)) return 1;

    if (!spawn_timer_terminal(self_exe, srv.sock_path, work, brk, rounds, &timer_pid)) {
        cleanup_server(&srv, 0);
        return 1;
    }

    printf("\033[2J\033[H"); // clear screen: \033[2J = ANSI escape code to clear the screen; \033[H Moves cursor to the home position (top-left)
    printf("waiting for timer to connect...\n");
    if(!ipc_server_accept(&srv)) {
        cleanup_server(&srv, timer_pid);
        return 1;
    }

    printf("timer connected\n");

    for (int i = 0; i < rounds; ++i) {
        printf("round %d/%d -- starting WORK on timer terminal...\n", i+1, rounds);
        if (!ipc_sendf(srv.conn_fd, "RUN_WORK %d", work)) break;
        if (!recv_expect(&srv, "WORK_DONE")) break;

        // last round
        if (i == rounds -1) {
            printf("last round done. signaling END_DAY...\n");
            // make timer play END_DAY_SOUND
            ipc_sendf(srv.conn_fd, "END_DAY");
            recv_expect(&srv, "END_ACK");

            printf("done!\n");
            break;  // leave the loop
        }

        printf("starting BREAK in this terminal...\n");
        countdown(brk, "break");
        play_sound(START_WORK_SOUND);
        if (!ipc_sendf(srv.conn_fd, "BREAK_DONE")) break;

    }

    cleanup_server(&srv, timer_pid);
    return 0;
}

// timer: connect to server, run work countdowns on RUN_WORK, play break sound after each work round, signal WORK_DONE, respond to END_DAY

int run_timer(const char *sock_path, int work, int brk, int rounds) {
    (void)brk; (void)rounds;  // work length is authoritative from controller

    IpcEndpoint cli = {0};
    if (!ipc_client_connect(&cli, sock_path)) return 1;
    printf("timer connected to %s\n", sock_path);

    char line[256], cmd[64];
    int arg = 0;

    while (ipc_recvline(cli.conn_fd, line, sizeof(line))) {
        if (sscanf(line, "%63s %d", cmd, &arg) >= 1) {
            if (strcmp(cmd, "RUN_WORK") == 0) {
                int minutes = (sscanf(line, "%*s %d", &arg) == 1) ? arg : work;
                printf("starting WORK for %d min...\n", minutes);
                countdown(minutes, "work");
                play_sound(START_BREAK_SOUND);
                if (!ipc_sendf(cli.conn_fd, "WORK_DONE")) break;
            } else if (strcmp(cmd, "BREAK_DONE") == 0) {
                printf("break finished on controller. waiting for next RUN_WORK...\n");
            } else if (strcmp(cmd, "END_DAY") == 0) {
                play_sound(END_DAY_SOUND);
                ipc_sendf(cli.conn_fd, "END_ACK");
                break;
            } else {
                fprintf(stderr, "unknown command: %s\n", line);
            }
        }
    }

    close(cli.conn_fd);
    return 0;
}