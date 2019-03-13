/**
 * Copyright (C) 2016 Spreadtrum Communications Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <time.h>
#include <pthread.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <sys/vfs.h>
#include <dlfcn.h>
#include <cutils/sockets.h>
#include "socket_util.h"
#include "autotest.h"

int create_udp_socket(int *fd, char *file) {
    int ret = -1;
    int sock = android_get_control_socket(file);
    if(sock < 0) {
        ret = socket_local_server(
          file, file[0] == '/' ? ANDROID_SOCKET_NAMESPACE_FILESYSTEM : ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_DGRAM);  /* DGRAM no need listen */
        if(ret < 0) {
            LOGD("socket_local_server %s failed: %s\n", file, strerror(errno));
        }
    }
    else {
        ret = dup(sock);
    }

    *fd = ret;
    return ret < 0 ? -1 : 0;
}

int connect_udp_socket(int *fd, char *name) {
    int ret;
    *fd = -1;
    ret = socket(PF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if(ret < 0) {
        LOGE("connect_socket_local_server_udp socket %s failed: %s\n", name, strerror(errno));
        return -1;
    }

    struct sockaddr_un  un;
    memset(&un, 0, sizeof(struct sockaddr_un));
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, name);

    if(connect(ret, (struct sockaddr *) &un, sizeof(struct sockaddr_un)) < 0) {
        close(ret);
        LOGE("connect_socket_local_server_udp connect %s failed: %s\n", name, strerror(errno));
        return -1;
    }

    *fd = ret;
    return 0;
}

int create_socket(int *fd, char *file) {
    *fd = socket_local_server(file, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if(*fd < 0) {
        LOGE("open %s failed: %s\n", file, strerror(errno));
        return -1;
    }

    return 0;
}

int connect_socket(int *fd, char *name) {
    *fd = socket_local_client(name, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if(*fd < 0) {
        LOGE("open %s failed: %s\n", name, strerror(errno));
        return -1;
    }

    return 0;
}

int accept_client(int fd) {
    struct sockaddr addr;
    socklen_t       addrlen = sizeof addr;
    return accept(fd, &addr, &addrlen);
}
