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
#include <sys/reboot.h>
#include <cutils/android_reboot.h>

#include "sprd_fts_type.h"
#include "sprd_fts_log.h"

#define AT_EMMCDDRSIZE "AT+EMMCDDRSIZE"
#define AT_RSP "+EMMCDDRSIZE: "

#define EMMC_SIZE_PATH   "/sys/block/mmcblk0/size"
#define DDR_SIZE_PATH    "/proc/sprd_dmc/property"

enum{
    SIZE_DDR,
    SIZE_EMMC,

    SIZE_NONE
};

long getSize(int type){
    int fd;
    char buffer[64]={0};
    char *endptr = NULL;
    char *path = NULL;

    if (type == SIZE_DDR){
        path = DDR_SIZE_PATH;
    }else if(type == SIZE_EMMC){
        path = EMMC_SIZE_PATH;
    }else{
        return 0;
    }

    fd = open(path,O_RDONLY);
    if(fd < 0){
        ENG_LOG("open %s fail\n", path);
        return 0;
    }
    if(read(fd,buffer,sizeof(buffer)) <= 0){
        ENG_LOG("read %s fail\n", path);
        return 0;
    }
    ENG_LOG("read %s succ: size = %s", path, buffer);
    close(fd);
    return strtoul(buffer,&endptr,0);;
}

static int getEmmcDDRSize_handle(char *buff, char *rsp)
{
    char *ptr = NULL;
    char cmd_buf[256] = {0};
    int ret = -1;
    if (NULL == buff)
    {
        ENG_LOG("%s,null pointer", __FUNCTION__);
        sprintf(rsp, "\r\n%sERROR\r\n", AT_RSP);
        return rsp != NULL ? strlen(rsp) : 0;
    }

    if(buff[0] == 0x7e)
    {
        ptr = buff + 1 + sizeof(MSG_HEAD_T);
    }
    else
    {
        ptr = strdup(buff);
    }

    ENG_LOG("%s ptr = %s", __FUNCTION__, ptr);
    if (strncasecmp(ptr, AT_EMMCDDRSIZE,strlen(AT_EMMCDDRSIZE)) == 0){
        long szEmmc = 0;
        long szDDR = 0;
        char buff[32] = {0};
        sprintf(rsp, "\r\n%s", AT_RSP);

        szEmmc = getSize(SIZE_EMMC);
        memset(buff, 0, sizeof(buff));
        sprintf(buff, "%4.2f GB", ceil(szEmmc/2/1024/1024));
        strcat(rsp, buff);

        strcat(rsp, "+");

        szDDR = getSize(SIZE_DDR);
        memset(buff, 0, sizeof(buff));
        if (szDDR > 1024){
            sprintf(buff, "%4.2f GB", ((float)szDDR)/1024);
        }else{
            sprintf(buff, "%4.2f MB", ((float)szDDR));
        }
        strcat(rsp, buff);
        strcat(rsp, "\r\n");

    }else{
        sprintf(rsp, "\r\n%sERROR\r\n", AT_RSP);
    }

    return strlen(rsp);
}

void register_this_module_ext(struct eng_callback *reg, int *num)
{
    int moudles_num = 0;
    ENG_LOG("register_this_module_ext :libapcomm");

    sprintf((reg + moudles_num)->at_cmd, "%s", AT_EMMCDDRSIZE);
    (reg + moudles_num)->eng_linuxcmd_func = getEmmcDDRSize_handle;
    moudles_num++;

    *num = moudles_num;
    ENG_LOG("register_this_module_ext: %d - %d",*num, moudles_num);
}
