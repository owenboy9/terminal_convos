#ifndef CHAT_H
#define CHAT_H

#include "ipc.h"
#include <stddef.h>

// send msg over an established ipc conn
int chat_send(IpcEndpoint *ep, const char *msg);

// read msg, store in buf
int chat_recv(IpcEndpoint *ep, char *buf, size_t n);

#endif // CHAT_H