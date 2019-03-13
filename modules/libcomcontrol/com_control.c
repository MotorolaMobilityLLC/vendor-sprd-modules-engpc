#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "sprd_fts_type.h"
#include "sprd_fts_log.h"

// return : rsp true length
int open_diag_port (char *req, char *rsp){

   ENG_LOG("dylib :enter function test :AT+DIAGOPEN!");
   property_set("persist.vendor.sys.modem.diag", ",gser");
   sprintf(rsp, "%s", "AT+DIAGOPEN open");
   return 0;
}
// return : rsp true length
int close_diag_port (char *req, char *rsp){
   ENG_LOG("dylib test :enter function test :AT+DIAGCLOSE!");
   property_set("persist.vendor.sys.modem.diag", "none");
sprintf(rsp, "%s", "AT+DIAGCLOSE close");
   return 0;
}

//最多添加32条AT指令
//num：实际注册的AT指令条数
void register_this_module_ext(struct eng_callback *reg, int *num)
{
    int moudles_num = 0;
    ENG_LOG("register_this_module_ext :dllibcomcontrol");

    //1st command
    sprintf(reg->at_cmd, "%s", "AT+DIAGOPEN");
    reg->eng_linuxcmd_func = open_diag_port;
    moudles_num++;

    //2nd command
    sprintf((reg+1)->at_cmd, "%s", "AT+DIAGCLOSE");
    (reg+1)->eng_linuxcmd_func = close_diag_port;
    moudles_num++;

    //3rd command
    //...

    *num = moudles_num;
    ENG_LOG("register_this_module_ext: %d - %d",*num, moudles_num);
}
