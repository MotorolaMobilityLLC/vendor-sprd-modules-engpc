#ifndef ENG_SOCKET_H
#define ENG_SOCKET_H

#include <cutils/log.h>

#ifdef __cplusplus
extern "C"
{
#endif
int create_udp_socket(int *fd, char *file);
int connect_udp_socket(int *fd, char *name);
int create_socket(int *fd, char *file);
int connect_socket(int *fd, char *name);
int accept_client(int fd);
#ifdef __cplusplus
}
#endif
#endif
