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

#include "eng_modules.h"
#include "engopt.h"
#include "eng_diag.h"

#define AT_AUTODLOADER "AT+SPREF=\"AUTODLOADER\""

void* autodloader_thread(void* arg) {
    sleep(2);
    android_reboot(ANDROID_RB_RESTART2, 0, "autodloader");
    return NULL;
}

static int dloader_handle(char *buff, char *rsp)
{
    char *ptr = NULL;
    char cmd_buf[256] = {0};
    int ret = -1;
    if (NULL == buff)
    {
        ALOGE("%s,null pointer", __FUNCTION__);
        sprintf(rsp, "\r\nERROR\r\n");
        return rsp != NULL ? strlen(rsp) : 0;
    }

    ptr = buff + 1 + sizeof(MSG_HEAD_T);
    ENG_LOG("%s ptr = %s", __FUNCTION__, ptr);
    if (strncasecmp(ptr, AT_AUTODLOADER,strlen(AT_AUTODLOADER)) == 0){
        //android_reboot(ANDROID_RB_RESTART2, 0, "autodloader");
        pthread_attr_t attr;
        pthread_t pthread;
        pthread_attr_init(&attr);
        pthread_create(&pthread, &attr, autodloader_thread, NULL);
        sprintf(rsp, "\r\nOK\r\n");
    }else{
        sprintf(rsp, "\r\nERROR\r\n");
    }

    return strlen(rsp);
}

void register_this_module_ext(struct eng_callback *reg, int *num)
{
    int moudles_num = 0;
    ENG_LOG("register_this_module_ext :libdloader");

    sprintf((reg + moudles_num)->at_cmd, "%s", AT_AUTODLOADER);
    (reg + moudles_num)->eng_linuxcmd_func = dloader_handle;
    moudles_num++;

    *num = moudles_num;
    ENG_LOG("register_this_module_ext: %d - %d",*num, moudles_num);
}
