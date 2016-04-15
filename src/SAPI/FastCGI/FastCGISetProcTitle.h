#ifndef UTIL_LINUX_SETPROCTITLE_H
#define UTIL_LINUX_SETPROCTITLE_H

#ifdef __cplusplus
extern "C"
{
#endif

void initproctitle (int argc, char **argv);
void setproctitle (const char *prog);

#ifdef __cplusplus
}
#endif
#endif