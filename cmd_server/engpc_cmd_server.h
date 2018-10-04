#ifndef ENG_CMD_SERVER_H
#define ENG_CMD_SERVER_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "socket_util.h"
#include "autotest.h"
#include "chnmgr.h"

#define SOCKET_SERVER_NAME "engpc_cmd_server"
#define MAX_BUF_LEN 4096
static int SOCKET_SERVER_FD;

int cmd_server_init(void);

#ifdef __cplusplus
}
#endif
#endif
