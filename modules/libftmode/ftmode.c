#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "sprd_fts_type.h"
#include "sprd_fts_diag.h"
#include "sprd_fts_log.h"

#include "miscdata.h"

#define MAX_CMDLIEN_LEN 1024
#define MAX_PROP_LEN    1024

#define MODE_CALI           "+TESTMODE: \"cali\""
#define MODE_POST_CALI      "+TESTMODE: \"post cali\""
#define MODE_AUTOTEST       "+TESTMODE: \"autotest\""
#define MODE_NORMAL         "+TESTMODE: \"normal\""
#define MODE_UNKNOWN        "+TESTMODE: \"unknown\""


#define TESTMODE_OFFSET (9*1024+32)

#define UBOOT_TESTMOD_CHECKSUM 0x53464d00

static char testmode_rsp[]={0x7E, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0xFE, 0xFF, 0x7E};


int isspace(char c){
    if (c == '\r' || c == '\n' || c == ' '){
        return 1;
    }

    return 0;
}

char* trim(char *str){
    char *end,*sp,*ep;
    int len;
    sp = str;
    end = str + strlen(str) - 1;
    ep = end;

    while(sp<=end && isspace(*sp))
        sp++;
    while(ep>=sp && isspace(*ep))
        ep--;
    len = (ep < sp) ? 0:(ep-sp)+1;
    sp[len] = '\0';
    return sp;
}

/* Parse one parameter which is before a special char for string.
 * buf:[IN], string data to be parsed.
 * gap:[IN], char, get value before this charater.
 * value:[OUT] parameter value
 * return length of parameter
 */
int cali_parse_one_para(char* buf, char gap, int* value) {
    int len = 0;
    char* ch = NULL;
    char str[10] = {0};

    if (buf != NULL && value != NULL) {
        ch = strchr(buf, gap);
        if (ch != NULL) {
            len = ch - buf;
            strncpy(str, buf, len);
            *value = atoi(str);
        }
    }

    return len;
}

int getTestMode(char *req, char *rsp){
    int fd = 0, ret = 0, len = 0;
    char cmdline[MAX_CMDLIEN_LEN] = {0};
    char prop[MAX_PROP_LEN] = {0};
    int mode = 0;
    int freq = 0;
    int device = 0;
    char* str = NULL;

    ENG_LOG("%s begin", __FUNCTION__);

    strcpy(rsp, MODE_NORMAL);

    fd = open("/proc/cmdline", O_RDONLY);
    if (fd >= 0) {
        if ((ret = read(fd, cmdline, sizeof(cmdline) - 1)) > 0) {
            ENG_LOG("eng_pcclient: cmdline %s\n", cmdline);
            /*cali or post cali*/
            str = strstr(cmdline, "calibration=");
            if (str != NULL) {
                str = strchr(str, '=');
                if (str != NULL) {
                    str++;
                    /*get calibration mode*/
                    len = cali_parse_one_para(str, ',', &mode);
                    if (len > 0) {
                        str = str + len + 1;
                        /*get calibration freq*/
                        len = cali_parse_one_para(str, ',', &freq);
                        /*get calibration device*/
                        str = str + len + 1;
                        len = cali_parse_one_para(str, ' ', &device);

                        if (freq != 0) {
                            strcpy(rsp, MODE_POST_CALI);
                        }else{
                            strcpy(rsp, MODE_CALI);
                        }
                    }
                }
            }

            //autotest
            str = strstr(cmdline, "androidboot.mode=autotest");
            ENG_LOG("%s: str: %s", __FUNCTION__, str);
            if (str != NULL) {
                strcpy(rsp, MODE_AUTOTEST);
            }
        }
        close(fd);
    }

    return strlen(rsp);
}

bool isdigitstr(char *str)
{
    return strspn(str, "0123456789")==strlen(str);
}

int setTestMode(char *req, char *rsp){
    int ret = 0;
    char* ptr = NULL;
    char* strNum = NULL;
    int len = 0;
    if(req[0] == 0x7e)
    {
        int datalen = ((MSG_HEAD_T*)(req+1))->len;
        ptr = req + 1 + sizeof(MSG_HEAD_T);
        *(req+datalen+1) = 0;
    }
    else
    {
        ptr = strdup(req);
        len = strlen(req);
    }
    ENG_LOG("%s: ptr = %s", __FUNCTION__, ptr);
    ptr = ptr+strlen("AT+SETTESTMODE=");
    strNum = trim(ptr);
    if (strNum != NULL && isdigitstr(strNum)){
        int val = atoi(strNum);
        if (val < 0 || val > 0xFF){
            ret = -2;
        }else{
            val += UBOOT_TESTMOD_CHECKSUM;
            if (0 != eng_write_miscdata_with_offset(TESTMODE_OFFSET, &val, 4)){
                ret = -3;
                ENG_LOG("write offset fail");
            }
        }
    }else{
        ret = -1;
    }

    if (ret == 0){
        sprintf(rsp, "+TESTMODE: OK");
    }else{
        sprintf(rsp, "+TESTMODE: ERROR %d", ret);
    }

    return strlen(rsp);
}

int testmodeRsp(char *buf, int len, char *rsp, int rsplen){
    memcpy(rsp, testmode_rsp, sizeof(testmode_rsp));
    return sizeof(testmode_rsp);
}

void register_this_module_ext(struct eng_callback *reg, int *num)
{
    int i = 0;
    int moudles_num = 0;
    ENG_LOG("register_this_module_ext :libftmode");

    sprintf(reg->at_cmd, "%s", "AT+GETTESTMODE?");
    reg->eng_linuxcmd_func = getTestMode;
    moudles_num++;

    sprintf((reg+moudles_num)->at_cmd, "%s", "AT+SETTESTMODE=");
    (reg+moudles_num)->eng_linuxcmd_func = setTestMode;
    moudles_num++;

    for(i = 0x80; i < 0x9F; i++){
        (reg+moudles_num)->type = 0xFE;
        (reg+moudles_num)->subtype = i;
        (reg+moudles_num)->eng_diag_func = testmodeRsp;
        moudles_num++;
    }

    *num = moudles_num;
    ENG_LOG("register_this_module_ext: %d - %d",*num, moudles_num);
}
