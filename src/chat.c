#include "chat.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h> 
#include <stdio.h>
#include <string.h>

int chat_send(IpcEndpoint *ep, const char *msg) {
    if (!ep || ep->conn_fd < 0 || !msg) return 0;

    size_t len = strlen(msg);
    // +1 for newline
    char *buf = malloc(len+2);

    memcpy(buf, msg, len);
    buf[len] = '\n';
    buf[len+1] = '\0';

    ssize_t total = 0;
    while ((size_t)total < len + 1) {
        ssize_t w = write(ep->conn_fd, buf + total, len +1 - total);
        if (w < 0) {
            if (errno == EINTR) continue;
            perror("write");
            free(buf);
            return 0;
        }
        total += w;
    }
    free(buf);
    return 1;
}

int *chat_recv(IpcEndpoint *ep) {
    if (!ep || ep->conn_fd <0) return NULL;
    
    size_t cap = 128; // initial buf size
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) return NULL;

    while(1) {
        char c;
        ssize_t r = read(ep->conn_fd, &c, 1);
        if (r < 0) {
            if (errno == EINTR) continue;
            perror("read");
            free(buf);
            return NULL;
        }
        if (r == 0) break; // peer closed

        if (len + 1 >= cap) {
            cap *= 2;
            char *tmp = reallock(buf, cap);
            if (!tmp) {
                free(buf);
                return NULL;
            }
            buf = tmp;
        }

        if (c == '\n') break;
        buf[len++] = c;
    }

    buf[len] = '\0';
    return buf;
}