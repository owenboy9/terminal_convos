#ifndef ROLES_H
#define ROLES_H

int run_controller(const char *self_exe, int work, int brk, int rounds);
int run_timer(const char *sock_path, int work,  int brk, int rounds);

#endif // ROLES_H