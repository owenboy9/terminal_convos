#define _GNU_SOURCE
#include "ipc.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

int ipc_server_start(IpcEndpoint *ep) {

    // make sure ep->dir_path has a valid mkdtemp template
    snprintf(ep->dir_path, sizeof(ep->dir_path), "/tmp/pomo.XXXXXX");
    if (!mkdtemp(ep->dir_path)) {
        perror("mkdtemp");
        return 0;
    }
    
    strncpy(ep->sock_path, ep->dir_path, sizeof(ep->sock_path) - 1);
    ep->sock_path[sizeof(ep->sock_path) -1] = '\0';  // ensure null-termination

    if (strlen(ep->sock_path) + strlen("/pomo.sock") < sizeof(ep->sock_path)) {
        strncat(ep->sock_path, "/pomo.sock", sizeof(ep->sock_path) - strlen(ep->sock_path) -1);
    } else {
        fprintf(stderr, "socket path too long!\n");
        return 0;
    }

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) { perror("socket"); return 0;}

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, ep->sock_path, sizeof(addr.sun_path)-1);

    // avoid leftover socket files
    unlink(ep->sock_path);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(fd);
        return 0;
    }

    if (listen(fd, 1) == -1) {
        perror("listen");
        close(fd);
        unlink(ep->sock_path);
        return 0;
    }

    ep->listen_fd = fd;
    ep->conn_fd = -1;  // not connected yet
    return 1;
}

int ipc_server_accept(IpcEndpoint *ep) {
    int cfd = accept(ep->listen_fd, NULL, NULL);
    if (cfd == -1) { perror ("accept"); return 0; }
    ep->conn_fd = cfd;
    return 1;
}

void ipc_server_cleanup(IpcEndpoint *ep) {
    if (ep->conn_fd >= 0) close(ep->conn_fd);
    if (ep->listen_fd >= 0) close(ep->listen_fd);
    if (ep->dir_path[0]) rmdir(ep->dir_path);
    memset(ep, 0, sizeof(*ep));
}

int ipc_client_connect(IpcEndpoint *ep, const char *sock_path) {
    memset(ep, 0, sizeof(*ep));
    strncpy(ep->sock_path, sock_path, sizeof(ep->sock_path)-1);
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) { perror("socket"); return 0; }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path)-1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(fd);
        return 0;
    }
    ep->conn_fd = fd;
    return 1;
}

int ipc_sendf(int fd, const char *ftm, ...) {
    char buf[256];
    va_list ap;
    va_start(ap, ftm);
    int n = vsnprintf(buf, sizeof(buf)-2, ftm, ap);
    va_end(ap);
    if (n<0) return 0;
    buf[n++] = '\n';  // append newline
    ssize_t w = write(fd, buf, n);
    return (w == n) ? 1 : 0;
}

int ipc_recvline(int fd, char *buf, size_t n) {
    size_t used = 0;
    while (used +1 <n ) {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r == 0) return 0;  // peer closed
        if (r < 0) { if (errno == EINTR) continue; perror("read"); return 0;}
        if (c == '\n') { buf[used] = '\0'; return 1; }
        buf[used++] = c;
    }
    buf[n-1] = '\0';
    return 1;  // truncated line
}