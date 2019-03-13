/**
 * Copyright (C) 2016 Spreadtrum Communications Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <poll.h>

#include "engpc_cmd.h"
#include "engpc_cmd_parser.h"

enum loglevel {
    LOG_ERROR,
    LOG_CRITICAL,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
};
int debug_level = LOG_DEBUG;
#ifdef ANDROID

#include "cutils/log.h"
#include "cutils/properties.h"
#include <cutils/sockets.h>
#define ___log_printf_trace___ SLOGW
#define ___log_printf___ printf
#else
#define ___log_printf_trace___(x...)
#define ___log_printf___ printf
#endif
#define cli_trace(msg...)  ___log_printf_trace___(msg)
#define cli_printf(msg...) ___log_printf___(msg)
#define cli_printf_debug(l, msg...) if (debug_level >= l) cli_printf(msg)
#define cli_info(msg...) cli_printf_debug(LOG_INFO, "log<info> "msg)
#define cli_warn(msg...) cli_printf_debug(LOG_WARN, "log<warn> "msg)
#define cli_critical(msg...) cli_printf_debug(LOG_CRITICAL, "log<critical> "msg)
#define ARRAY_LEN(A) (sizeof(A)/sizeof((A)[0]))

#ifdef ANDROID
int connect_socket_local_server(char *name) {
    int fd = socket_local_client(name, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if (fd < 0) {
        LOGE("%s open %s failed: %s\n", __func__, name, strerror(errno));
        return -1;
    }
    return fd;
}
#else
int connect_socket_local_server(char *name) {
    struct sockaddr_un address;
    int fd;
    int namelen;
    /* init unix domain socket */
    fd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (fd < 0) {
        LOGE("%s open %s failed: %s\n", __func__, name, strerror(errno));
        return -1;
    }

    namelen = strlen(name);
    /* Test with length +1 for the *initial* '\0'. */
    if ((namelen + 1) > (int)sizeof(address.sun_path)) {
        cli_critical("%s %s length is too long\n", __func__, name);
        close(fd);
        return -1;
    }
    /* Linux-style non-filesystem Unix Domain Sockets */
    memset(&address, 0, sizeof(address));
    address.sun_family = PF_LOCAL;
    strcpy(&address.sun_path[1], name); /* local abstract socket server */

    if (connect(fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        LOGE("%s connect %s failed: %s\n", __func__, name, strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}
#endif

int usage()
{
    LOGD("usage: adb shell factory_cli [COMMAND]\n");
    return -1;
}

int main(int argc, char *argv[]) {
    int fd;
    int i = 0;
    char buf[1024];
    char ori_buf[1024];
    char val_buf[1024];
    int buf_size = sizeof buf;
    int ret;
    int use_hidl_socket = 0;
    //char *p, *pmax;
    struct pollfd pfd[2];
    int forced_exit = 0;

    char output_cmd[BUF_LEN] = {0};

    for (i = 0; i < argc; i++) {
      LOGD("%s: i=%d,cmd=%s",__FUNCTION__, i, argv[i]);
    }
    //Check arg
    if(argc < 2){
        return usage();
    }

    fd = connect_socket_local_server("hidl_common_socket");
    LOGD("connect -> hidl_common_socket server fd:%d", fd);
    if(fd < 0){
	    fd = connect_socket_local_server(SOCKET_SERVER_NAME);
	    LOGD("connect -> %s server fd:%d", SOCKET_SERVER_NAME, fd);
	    use_hidl_socket = 0;
    }else{
        use_hidl_socket = 1;
    }
    if (fd < 0){
       LOGD("Error %s: i=%d,cmd=%s",__FUNCTION__, i, argv[i]);
       return -1;
    }

    //parse config
    parse_config();

    //Check does support by sprd default cmd.
    if(parse_support_cmd(argc,argv,output_cmd) == 1){
        snprintf(ori_buf, strlen(output_cmd) + 1, "%s", output_cmd);
    }else{
        if(argc > 2){
            for(i=2;i<argc;i++){
                if(i > 2){
                  strcat(val_buf,",");
                }
                strcat(val_buf,argv[i]);
            }
            LOGD("%s: val_buf:%s", __FUNCTION__, val_buf);
            snprintf(ori_buf, strlen(argv[1]) + strlen(val_buf) + 1 + 1, "%s=%s", argv[1], val_buf);
        }else{
            snprintf(ori_buf, strlen(argv[1]) + 1, "%s", argv[1]);
        }
    }
    LOGD("%s: Run cmd :ori_buf:%s", __FUNCTION__, ori_buf);

    /** Use fixed socket name
    p = buf;
    pmax = buf + buf_size;
    argv[0] = SOCKET_SERVER_NAME;
    for (i = 1; i < argc; i++) {
        p += snprintf(p, pmax - p, "%s ", argv[i]);
    }
    p += snprintf(p, pmax - p, "\n");
    LOGD("cli -> server :%s", buf);
    */

    if(use_hidl_socket == 1){
       snprintf(buf, sizeof(buf), "%s %s", SOCKET_SERVER_NAME , ori_buf);
    }else{
       snprintf(buf, sizeof(buf), "%s" , ori_buf);
    }
    LOGD("connect -> server buf:%s,use_hidl_socket:%d", buf,use_hidl_socket);

    write(fd, buf, strlen(buf));
    cli_trace("%s", buf);

    pfd[0].fd = fd;
    pfd[0].events = POLLIN;

    for (;;) {
        if (poll(pfd, 1, -1) <= 0) {
            LOGE("poll");
            continue;
        }
        if (pfd[0].revents) {
            ret = read(pfd[0].fd, buf, buf_size);
            if (ret > 0) {
                if (ret >= 20 && strncmp(&buf[ret-20], "____cli____exit____\n", 20) == 0) {
                    ret -= 20;
                    buf[ret] = 0;
                    forced_exit = 1;
                }
                //forced_exit = 1;
                write(STDOUT_FILENO, buf, ret);
                if (forced_exit) {
                    LOGD("cli is disconnected by service\n");
                    //exit(0);
                    break;
                }
            } else if (ret == 0) {
                LOGD("server closed\n");
                //exit(0);
                break;
            } else {
                LOGE("cli read failed: %s\n", strerror(errno));
               //exit(0);
               break;
            }
        }
    }
    if(fd > 0){
	close(fd);
    }

    return 0;
}
