#ifndef TERMSPAWN_H
#define TERMSPAWN_H

#include <stddef.h>
#include <sys/types.h>

int spawn_chat_terminal(const char *self_exe, const char *sock_path, const char *msg, pid_t *out_pid);

#endif // TERMSPAWN_H