#ifndef IPC_H
#define IPC_H

#include <stddef.h>

typedef struct {
    int listen_fd;  // server listening fd (controller side)
    int conn_fd;  // connected fd
    char dir_path[512];  // /tmp/pomo-XXXXXX
    char sock_path[512];  //  /tmp/pomodoro-XXXXXX/pomo.sock
} IpcEndpoint;

// controller (server) setup /teardown

int ipc_server_start(IpcEndpoint *ep);
int ipc_server_accept(IpcEndpoint *ep);
void ipc_server_cleanup(IpcEndpoint *ep);

// timer (client) connection
int ipc_client_connect(IpcEndpoint *ep, const char *sock_path);

// line protocol helpers (append '\n', read up to '\n')
int ipc_sendf(int fd, const char *ftm, ...);  // returns 1 on success
int ipc_recvline(int fd, char *buf, size_t n);  // returns 1 on success

#endif