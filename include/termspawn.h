#ifndef TERMSPAWN_H
#define TERMSPAWN_H

#include <stddef.h>
#include <sys/types.h>

int spawn_timer_terminal(const char *self_exe, const char *sock_path, int work_min, int break_min, int rounds, pid_t *out_pid);

#endif // TERMSPAWN_H