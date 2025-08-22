#include "chat.h"
#include <stdio.h>
#include <string.h>

int chat_send(IpcEndpoint *ep, const char *msg) {
    if (!ep || ep->conn_fd < 0 || !msg) return 0;
    // send newline-terminated msg
    return ipc_sendf(ep->conn_fd, "%s\n", msg);
}

int chat_recv(IpcEndpoint *ep, char *buf, size_t n) {
    if (!ep || ep->conn_fd <0 || !buf || n == 0) return 0;
    if (!ipc_recvline(ep->conn_fd, buf, n)) return 0;
    return 1;
}