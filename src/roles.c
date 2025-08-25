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
#include <sys/select.h>
#include <errno.h>


// helper for trimming trailing newlines
static void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len && s[len-1] == '\n') s[len-1] = '\0';
}

// full-duplex chat loop using select()
// - reads from STDIN and socket concurrently
// - prints peer messages as they arrive
// - sends your messages as you type
// returns 0 on clean exit, 1 on error
static int chat_loop(IpcEndpoint *ep, const char *name, const char *peer_name) {
    char inbuf[1024];

    printf("hey, %s! type /quit to exit.\n!", name);

    for (;;) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(ep->conn_fd, &rfds);

        int maxfd = (STDIN_FILENO > ep->conn_fd) ? STDIN_FILENO : ep->conn_fd;

        int rc = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (rc < 0) {
            if (errno == EINTR) continue;
            perror("select");
            return 1;
        }

        // socket ready, recv & print
        if (FD_ISSET(ep->conn_fd, &rfds)) {
            char *msg = chat_recv(ep);
            if (!msg) {
                fprintf(stderr, "\n%s disconnected.\n", peer_name);
                return 0;
            }

            // handle peer's quitting
            if (strcmp(msg, "/quit") == 0) {
                printf("\n%s left the chat\n", peer_name);
                free(msg);
                return 0;
            }

            // move to line start, print peer msg, then restore prompt
            printf("\r%s: %s\n", peer_name, msg);
            free(msg);
            //re-print prompt if user is typing
            printf("you> ");
            fflush(stdout);
        }

        // stdin ready -- read & send
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            if (!fgets(inbuf, sizeof(inbuf), stdin)) {
                // EOF on stdin, treat as quit
                (void)chat_send(ep, "/quit");
                return 0;
            }

            trim_newline(inbuf);
            if (strcmp(inbuf, "/quit") == 0) {
                (void)chat_send(ep, "/quit");
                return 0;
            }
            if (inbuf[0] == '\0') {
                // skip empty input
                printf("you> ");
                fflush(stdout);
                continue;
            }
            if (!chat_send(ep, inbuf)) {
                fprintf(stderr, "failed to send message\n");
                return 1;
            }
        }
    }
}

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

    if (!chat_send(&srv, name)) {
        fprintf(stderr, "failed to send name");
        ipc_server_cleanup(&srv);
        return 1;
    }

    char *peer_name = chat_recv(&srv);
    if (!peer_name) {
        fprintf(stderr, "failed to receive peer name\n");
        cleanup_server(&srv, term_pid);
        return 1;
    }

    printf("%s joined chat\n", peer_name);

    int res = chat_loop(&srv, name, peer_name);
    free(peer_name);
    close(srv.conn_fd);

    if (term_pid > 0) {
        int status;
        waitpid(term_pid, &status, 0);
    }
    ipc_server_cleanup(&srv);
    return res;
}

// 2nd terminal: connect to server, send / receive messages
int run_chat(const char *sock_path) {
    IpcEndpoint cli = {0};
    char name[56];
    prompt_user_name_2(name, sizeof(name));
    // controller mode

    if (!ipc_client_connect(&cli, sock_path)) return 1;
    printf("chat terminal connected to %s\n", sock_path);
    
    char *peer_name = chat_recv(&cli);
    if (!peer_name) {
        fprintf(stderr, "failed to receive terminal name\n");
        cleanup_server(&cli, 0);
        return 1;
    }

    if (!chat_send(&cli, name)) {
        fprintf(stderr, "failed to send your name");
        close(cli.conn_fd);
        return 1;
    }

    printf("%s joined chat\n", peer_name);

    // full-duplex chat
    int res = chat_loop(&cli, name, peer_name);
    free(peer_name);
    close(cli.conn_fd);
    return res;
}