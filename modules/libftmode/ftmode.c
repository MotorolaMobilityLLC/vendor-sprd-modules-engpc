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

#define MAX_CMDLIEN_LEN 3096


#define CALI_DPU_STOP "/sys/class/display/dispc0/cali_dpu_stop"
#define PANEL_SUSPEND "/sys/class/display/panel0/suspend"
#define CALI_DPHY_SUSPEND "/sys/class/display/dphy0/cali_dphy_suspend"
#define CALI_DSI_SUSPEND "/sys/class/display/dsi/cali_dsi_suspend"
#define DPMS_STATE "/sys/class/display/dispc0/dpms_state"
#define BRIGHTNESS "/sys/class/backlight/sprd_backlight/brightness"

#define AT_CMD_GETTESTMODE "AT+GETTESTMODE?"
#define AT_CMD_SETTESTMODE "AT+SETTESTMODE="
#define AT_LCDOFF "AT+LCD=OFF"

#define TESTMODE_OFFSET (9*1024+32)
#define UBOOT_TESTMOD_CHECKSUM 0x53464d00
#define CMDLINE_FIRST_MODE_FLAG "first_mode="
#define CMDLINE_CALI_MODE_FLAG "calibration="

typedef struct _MODEID_NAME{
    unsigned int id;
    char name[64];
}MODE_ID_NAME;

MODE_ID_NAME id_first_mode[]= {
    {0x00, "NORMAL"},
    {0x01, "GSMCAL"},
    {0x02, "GSMFT"},
    {0x03, "WCDMACAL"},
    {0x04, "WCDMAFT"},
    {0x05, "TDSCAL"},
    {0x06, "TDSFT"},
    {0x07, "LTECAL"},
    {0x08, "LTEFT"},
    {0x0B, "NRCAL"},
    {0x0C, "NRFT"},
    {0x0D, "NRMMWCAL"},
    {0x0E, "NRMMWFT"},
    {0x0F, "C2KCAL"},
    {0x10, "C2KFT"},
    {0x11, "BBAT"},
    {0x12, "NATIVEMMI"},
    {0x13, "APKMMI"},
    {0x14, "NBIOTCAL"},
    {0x15, "NBIOTFT"},
    {0x16, "UPT"}
};

MODE_ID_NAME id_cali_mode[]= {
    {0, "NORMAL"},
    {1, "GSMCAL"},
    {5, "GSMFT"},
    {7, "TDSCAL"},
    {8, "TDSFT"},
    {11, "WCDMACAL"},
    {12, "WCDMAFT"},
    {16, "LTECAL"},
    {17, "LTEFT"},
    {18, "C2KCAL"},
    {19, "C2KFT"},
    {24, "NRCAL"},
    {25, "NRFT"},
};

static char testmode_rsp[]={0x7E, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0xFE, 0xFF, 0x7E};

unsigned int findIdWithName(char* name){
    if (name == NULL) return 0xFF;
    ENG_LOG("name: %s", name);

    for(int i = 0; i < sizeof(id_first_mode)/sizeof(MODE_ID_NAME); i++){
        if (strcasecmp(id_first_mode[i].name, name) == 0){
            return id_first_mode[i].id;
        }
    }

    return 0xFF;
}

char* findNameWithId(int id, int modeFlag){
    ENG_LOG("id: 0x%x", id);
    int mode_num[] = {sizeof(id_first_mode)/sizeof(MODE_ID_NAME),
                      sizeof(id_cali_mode)/sizeof(MODE_ID_NAME)};
    MODE_ID_NAME* idName;
    if(modeFlag == 0){
      idName = id_first_mode;
    }else if(modeFlag == 1){
      idName = id_cali_mode;
    }

    for(int i = 0; i < mode_num[modeFlag]; i++){
        if (idName[i].id == id){
            return idName[i].name;
        }
    }

    return NULL;
}

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

int parse_key_value(char* buf, char* key, int flag_mode) {
    int len = 0;
    char value[8] = {0};
    char* str = NULL;
    char* gap = NULL;

    str = strstr(buf, key);
    if (str != NULL){
        str = strchr(str, '=');
        if (str != NULL){
            str++;
            if (str != NULL){
                if(flag_mode == 0){
                    gap=strchr(str, ' ');
                }else{
                    gap=strchr(str, ',');
                }
                if (gap != NULL){
                    len = gap - str;
                }else{
                    len = 2;
                }
                strncpy(value, str, len);
            }
        }
    }
   
    return strtol(value, NULL, flag_mode == 1 ? 10 : 16);
    
}

int getTestMode(char *req, char *rsp){
    int fd = 0, ret = 0, len = 0;
    char cmdline[MAX_CMDLIEN_LEN] = {0};
    unsigned int value = 0x00;
    char* str = NULL;
    char* mode = 0;
    int mode_flag = 0;

    ENG_LOG("%s ", __FUNCTION__);

    fd = open("/proc/cmdline", O_RDONLY);

    if (fd >= 0) {
        if ((ret = read(fd, cmdline, sizeof(cmdline) - 1)) > 0) {
            ENG_LOG("cmdline %s\n", cmdline);

            /*like this first_mode=11*/
            if(strstr(cmdline, CMDLINE_FIRST_MODE_FLAG)){
                value = parse_key_value(cmdline, CMDLINE_FIRST_MODE_FLAG, mode_flag);
            }else if(strstr(cmdline, CMDLINE_CALI_MODE_FLAG)){
                mode_flag=1;
                value = parse_key_value(cmdline, CMDLINE_CALI_MODE_FLAG, mode_flag);
            }
        }

        close(fd);
    }

    mode = findNameWithId(value, mode_flag);
    if (mode == NULL){
        sprintf(rsp, "+GETTESTMODE:0x%X", value);
    }else{
        sprintf(rsp, "+GETTESTMODE:%s", mode);
    }

    return strlen(rsp);
}

bool isdigitstr(char *str){
    return strspn(str, "0123456789AaBbCcDdEeFfXx")==strlen(str);
}

bool isHexstr(char* str){
    if (strlen(str) > 2 && strcasestr(str, "0x") != NULL){
        return true;
    }

    return false;
}

int setTestMode(char *req, char *rsp){
    int ret = 0;
    char* ptr = NULL;
    char* ptrTmp = NULL;
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
    }

    //ptrTmp = ptr;
    ENG_LOG("%s: ptr = %s", __FUNCTION__, ptr);
    ptrTmp = ptr+strlen(AT_CMD_SETTESTMODE);
    if (ptrTmp != NULL && (strNum = trim(ptrTmp)) != NULL){
        unsigned int val = 0;
        if ( isdigitstr(strNum) ){
            if (isHexstr(strNum)){
                val = strtol(strNum, NULL, 16);
            }else{
                val = strtol(strNum, NULL, 10);
            }
        }else{
            val = findIdWithName(strNum);
        }

        ENG_LOG("%s: val = 0x%X", __FUNCTION__, val);
        val += UBOOT_TESTMOD_CHECKSUM;
        if (0 != eng_write_miscdata_with_offset(TESTMODE_OFFSET, &val, 4)){
            ret = -3;
            ENG_LOG("write offset fail");
        }
    }else{
        ret = -1;
    }

    if (ret == 0){
        sprintf(rsp, "+SETTESTMODE:OK");
    }else{
        sprintf(rsp, "+SETTESTMODE: ERROR %d", ret);
    }

    if (req[0] != 0x7e && ptrTmp != NULL){
        free(ptrTmp);
    }

    return strlen(rsp);
}

int testmodeRsp(char *buf, int len, char *rsp, int rsplen){
    memcpy(rsp, testmode_rsp, sizeof(testmode_rsp));
    return sizeof(testmode_rsp);
}

int modeRsp_match(char* buff, int len){
    if (buff[7] == 0xFE && buff[8] >= 0x80 && buff[8] <= 0x9F){
        return 0;
    }else{
        return -1;
    }
}

static int set_cali_dpu_stop(int cali_dpu_stop, char *rsp)
{
    int ret = -1;
    int fd;
    char value[8] = {};
    fd = open(CALI_DPU_STOP, O_WRONLY);
    if (fd > 0) {
        sprintf(value, "%d",cali_dpu_stop);
        ret = write(fd, value, strlen(value));
        close(fd);
        if (ret < 0) {
            ENG_LOG("%s set dpu_stop(%d) fail:%d\n", __FUNCTION__, cali_dpu_stop, errno);
            sprintf(rsp, "+LCD:set cali_dpu_stop fail");
        }
    } else {
        ENG_LOG("%s open %s fail:%s\n", __FUNCTION__,
                        CALI_DPU_STOP, strerror(errno));
        sprintf(rsp, "+LCD:open dpu_stop fail");
    }

    return (ret < 0) ? -errno : 0;
}

static int set_panel_suspend(int suspend, char *rsp)
{
    int ret = -1;
    int fd;
    char value[8] = {};

    fd = open(PANEL_SUSPEND, O_WRONLY);
    if (fd > 0) {
        sprintf(value, "%d", suspend);
        ret = write(fd, value, strlen(value));
        close(fd);
        if (ret < 0) {
            ENG_LOG("%s set suspend(%d) fail:%d\n", __FUNCTION__, suspend, errno);
            sprintf(rsp, "+LCD:set suspend fail");
        }
    } else {
        ENG_LOG("%s open %s fail:%s\n", __FUNCTION__,
                        PANEL_SUSPEND, strerror(errno));
        sprintf(rsp, "+LCD:open suspend fail");
    }

    return (ret < 0) ? -errno : 0;
}

static int set_cali_dphy_suspend(int cali_dphy_suspend, char *rsp)
{
    int ret = -1;
    int fd;
    char value[8] = {};
    fd = open(CALI_DPHY_SUSPEND, O_WRONLY);
    if (fd > 0) {
        sprintf(value, "%d", cali_dphy_suspend);
        ret = write(fd, value, strlen(value));
        close(fd);
        if (ret < 0) {
            ENG_LOG("%s set cali_dphy_suspend(%d) fail:%d\n", __FUNCTION__, cali_dphy_suspend, errno);
            sprintf(rsp, "+LCD:set cali_dphy_suspend fail");
        }
    } else {
        ENG_LOG("%s open %s fail:%s\n", __FUNCTION__,
                        CALI_DPHY_SUSPEND, strerror(errno));
        sprintf(rsp, "+LCD:open cali_dphy_suspend fail");
    }

    return (ret < 0) ? -errno : 0;
}

static int set_cali_dsi_suspend(int cali_dsi_suspend, char *rsp)
{
    int ret = -1;
    int fd;
    char value[8] = {};

    fd = open(CALI_DSI_SUSPEND, O_WRONLY);
    if (fd > 0) {
        sprintf(value, "%d",cali_dsi_suspend);
        ret = write(fd, value, strlen(value));
        close(fd);
        if (ret < 0) {
            ENG_LOG("%s set cali_dsi_suspend(%d) fail:%d\n", __FUNCTION__, cali_dsi_suspend, errno);
            sprintf(rsp, "+LCD:set cali_dsi_suspend fail");
        }
    } else {
        ENG_LOG("%s open %s fail:%s\n", __FUNCTION__,
                        CALI_DSI_SUSPEND, strerror(errno));
        sprintf(rsp, "+LCD:open cali_dsi_suspend fail");
    }

    return (ret < 0) ? -errno : 0;
}

static int set_dpms_state(int dpms_state, char *rsp)
{
    int ret = -1;
    int fd;
    char value[8] = {};
    fd = open(DPMS_STATE, O_WRONLY);
    if (fd > 0) {
        sprintf(value, "%d", dpms_state);
        ret = write(fd, value, strlen(value));
        close(fd);
        if (ret < 0) {
            ENG_LOG("%s set dpms_state(%d) fail:%d\n", __FUNCTION__, dpms_state, errno);
            sprintf(rsp, "+LCD:set dpms_state fail");
        }
    } else {
        ENG_LOG("%s open %s fail:%s\n", __FUNCTION__,
                        DPMS_STATE, strerror(errno));
        sprintf(rsp, "+LCD:open dpms_state fail");
    }

    return (ret < 0) ? -errno : 0;
}

static int set_backlight_brightness(int brightness, char *rsp)
{
    int ret = -1;
    int fd;
    char value[8] = {};
    fd = open(BRIGHTNESS, O_WRONLY);
    if (fd > 0) {
        sprintf(value, "%d", brightness);
        ret = write(fd, value, strlen(value));
        close(fd);
        if (ret < 0) {
            ENG_LOG("%s set brightness(%d) fail:%d\n", __FUNCTION__, brightness, errno);
            sprintf(rsp, "+LCD:set brightness fail");
        }
    } else {
        ENG_LOG("%s open %s fail:%s\n", __FUNCTION__,
                        BRIGHTNESS, strerror(errno));
        sprintf(rsp, "+LCD:open brightness fail");
    }

    return (ret < 0) ? -errno : 0;
}

static int lcd_power(char *req, char *rsp)
{
    unsigned long brightness;
    char *ptr = NULL;
    int ret;

    if (NULL == req) {
        ENG_LOG("%s null pointer", __FUNCTION__);
        if(rsp != NULL) {
            sprintf(rsp, "+LCD:null req");
            return strlen(rsp);
        } else
            return 0;
    }

    ptr = req + 1 + sizeof(MSG_HEAD_T);
 if (strncasecmp(ptr, AT_LCDOFF, strlen(AT_LCDOFF)) == 0) {
        ret = set_cali_dpu_stop(0, rsp);
        if (ret) {
            sprintf(rsp, "+LCD:set cali_dpu_stop fail");
            return strlen(rsp);
        }
        sprintf(rsp, "+LCD:OK");

        ret = set_panel_suspend(1, rsp);
        if (ret) {
            sprintf(rsp, "+LCD:set panel suspend fail");
            return strlen(rsp);
        }
        sprintf(rsp, "+LCD:OK");

        ret = set_cali_dphy_suspend(1, rsp);
        if (ret) {
            sprintf(rsp, "+LCD:set cali_dphy_suspend fail");
            return strlen(rsp);
        }
        sprintf(rsp, "+LCD:OK");

        ret = set_cali_dsi_suspend(1, rsp);
        if (ret) {
            sprintf(rsp, "+LCD:set cali_dsi_suspend fail");
            return strlen(rsp);
        }
        sprintf(rsp, "+LCD:OK");

        ret = set_dpms_state(1, rsp);
        if (ret) {
            sprintf(rsp, "+LCD:set dpms fail");
            return strlen(rsp);
        }
        sprintf(rsp, "+LCD:OK");

        ret = set_backlight_brightness(0, rsp);
        if (ret) {
            sprintf(rsp, "+LCD:set brightness fail");
            return strlen(rsp);
        }
        sprintf(rsp, "+LCD:OK");
    } else
        sprintf(rsp, "+LCD:unknown cmd");

    return strlen(rsp);
}

void register_this_module_ext(struct eng_callback *reg, int *num)
{
    int i = 0;
    int moudles_num = 0;
    ENG_LOG("register_this_module_ext :libftmode");

    sprintf(reg->at_cmd, "%s", AT_CMD_GETTESTMODE);
    reg->eng_linuxcmd_func = getTestMode;
    moudles_num++;

    sprintf((reg+moudles_num)->at_cmd, "%s", AT_LCDOFF);
    (reg+moudles_num)->eng_linuxcmd_func = lcd_power;
    moudles_num++;

    sprintf((reg+moudles_num)->at_cmd, "%s", AT_CMD_SETTESTMODE);
    (reg+moudles_num)->eng_linuxcmd_func = setTestMode;
    moudles_num++;

    (reg+moudles_num)->eng_diag_func = testmodeRsp;
    (reg+moudles_num)->eng_cmd_match = modeRsp_match;
     moudles_num++;

    *num = moudles_num;
    ENG_LOG("register_this_module_ext: %d - %d",*num, moudles_num);
}
