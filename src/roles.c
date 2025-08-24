#include "roles.h"
#include "ipc.h"
#include "ui.h"
#include "chat.h"
#include "termspawn.h"
#include "sound.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <stddef.h>


// cleanup helper
static void cleanup_server(IpcEndpoint *srv, pid_t term_pid) {
    if (term_pid > 0) {
        int status;
        waitpid(term_pid, &status, 0);
    }
    ipc_server_cleanup(srv);
}

// controller: create server, spawn terminal, accept connection, send/receive chat messages

int run_controller(const char *self_exe) {
    IpcEndpoint srv = {0};
    pid_t term_pid = 0;

    char name[56];
    // controller mode
    prompt_user_name_1(name, sizeof(name));

    if (!ipc_server_start(&srv)) return 1;

    if (!spawn_chat_terminal(self_exe, srv.sock_path, NULL, &term_pid)) {
        cleanup_server(&srv, 0);
        return 1;
    }

    printf("\033[2J\033[H"); // clear screen: \033[2J = ANSI escape code to clear the screen; \033[H Moves cursor to the home position (top-left)
    printf("waiting for chat terminal to connect...\n");
    if(!ipc_server_accept(&srv)) {
        cleanup_server(&srv, term_pid);
        return 1;
    }

    printf("chat terminal connected\n");

    chat_send(&srv, name);
    char *peer_name = chat_recv(&srv);
    if (!peer_name) {
        fprintf(stderr, "failed to receive terminal name\n");
        cleanup_server(&srv, term_pid);
        return 1;
    }

    printf("%s joined chat\n", peer_name);

    char buf[1024];
    while (1) {
        printf("%s> ", name);
        fflush(stdout);

        // read user input
        if(!fgets(buf, sizeof(buf), stdin)) break;
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';

        // send msg to terminal
        if (!chat_send(&srv, buf)) {
            fprintf(stderr, "failed to send your msg\n");
            break;
        }

        // recv reply
        char *reply = chat_recv(&srv);
        if (!reply) {
            fprintf(stderr, "terminal disconnected\n");
            break;
        }
        printf("%s is saying: %s\n", peer_name, reply);
        free(reply);
    }

    cleanup_server(&srv, term_pid);
    return 0;
}

// 2nd terminal: connect to server, send / receive messages
int run_chat(const char *sock_path) {
    IpcEndpoint cli = {0};
    char name[56];
    prompt_user_name_2(name, sizeof(name));
    // controller mode

    if (!ipc_client_connect(&cli, sock_path)) return 1;

    printf("chat terminal connected to %s\n", sock_path);
    chat_send(&cli, name);
    char *peer_name = chat_recv(&cli);
    if (!peer_name) {
        fprintf(stderr, "failed to receive terminal name\n");
        cleanup_server(&cli, 0);
        return 1;
    }

    printf("%s joined chat\n", peer_name);

    char buf[1204];

    while (1) {
        char *msg = chat_recv(&cli);
        if (!msg) {
            fprintf(stderr, "chat disconnected\n");
            break;
        }

        printf("%s is saying: %s\n", peer_name, msg);
        free(msg);

        printf("%s> ", name);
        fflush(stdout);

        if (!fgets(buf, sizeof(buf), stdin)) break;
        size_t len = strlen(buf);
        
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';

        if (!chat_send(&cli, buf)) {
            fprintf(stderr, "failed to send msg\n");
            break;
        }
        play_sound(NEW_MESSAGE);
    }

    close(cli.conn_fd);
    return 0;
}