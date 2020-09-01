#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "sprd_fts_type.h"
#include "sprd_fts_log.h"
#include "sprd_fts_diag.h"
#include <cutils/properties.h>
#include <sys/socket.h>
#include <cutils/sockets.h>


static int sendCmd(char* cmd) {
    int fd = -1;
    char result[512] = {0};
    fd = socket_local_client("ylog_cli_cmd", ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if (fd < 0) {
        ENG_LOG("socket_local_client ylog_cli_cmd failed");
        return -1;
    }
    if (send(fd, cmd, strlen(cmd), 0) == -1) {
        ENG_LOG("send ylog_cli_cmd error");
    }
    if (recv(fd, result, sizeof(result), 0) < 0) {
        ENG_LOG("recv  ylog_cli_cmd  error");
    }
    if (fd >= 0) {
        close(fd);
    }
    return 0;
}

static int start_ylog_to_pc(char *req, char *rsp)
{
    ENG_LOG("start ylog to pc function");
    sendCmd("startylog2pc\n");
    return 0;
}

static int stop_ylog_to_pc(char *req, char *rsp)
{
    ENG_LOG("stop ylog to pc function");
    sendCmd("stopylog2pc\n");
    return 0;
}

void register_this_module_ext(struct eng_callback *reg, int *num)
{
    int moudles_num = 0;
    ENG_LOG("register_this_module_ext :libylogtopc");

    //start
    reg->type = 0x70; //main cmd
    reg->subtype = 0x00; //sub cmd
    reg->eng_diag_func = start_ylog_to_pc;
    moudles_num++;

    //stop
    (reg+1)->type = 0x70; //main cmd
    (reg+1)->subtype = 0x01; //sub cmd
    (reg+1)->eng_diag_func = stop_ylog_to_pc;
    moudles_num++;

    *num = moudles_num;
    ENG_LOG("register_this_module_ext: %d - %d",*num, moudles_num);
}
