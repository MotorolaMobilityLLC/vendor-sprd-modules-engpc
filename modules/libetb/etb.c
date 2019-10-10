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

#define DEV_ETB_PATH "/dev/tmc_etb"

#define ETB_DUMP_CMD        0x0033
#define ETB_DUMP_START      0x00
#define ETB_DUMP_DUMPING    0x01
#define ETB_DUMP_END        0x02

#define ETB_DUMP_BUFF (16*1024)

typedef struct ST_ETB_DUMP_HEAD{
    short   cmd;
    short   len;
}ETB_DUMP_HEAD, *LPETB_DUMP_HEAD;

typedef struct ST_ETB_DUMP_DATA{
    int  type;
    char data;
}ETB_DUMP_DATA, *LPETB_DUMP_DATA;


static DYMIC_WRITETOPC_FUNC g_func[WRITE_TO_MAX] = {NULL};

int get_fw_ptr(DYMIC_WRITETOPC_FUNC * write_interface_ptr){
    ENG_LOG("%s", __FUNCTION__);

    for(int i = 0; i < WRITE_TO_MAX; i++) {
        g_func[i] = write_interface_ptr[i];
        if(g_func[i] != NULL)
            ENG_LOG("%s ad 0x%x, i %d", __FUNCTION__, g_func[i], i);
    }

    return 0;
}

int translate_packet(char *buf, int len){
    int i, j;
    char tmp;

    ENG_LOG("%s: len=%d", __FUNCTION__, len);

    for (i = 0; i < len; i++) {
        if ((buf[i] == 0x7d) || (buf[i] == 0x7e)) {
            tmp = buf[i] ^ 0x20;
            buf[i] = 0x7d;
            for (j = len; j > i + 1; j--) {
                buf[j] = buf[j - 1];
            }
            buf[i + 1] = tmp;
            len++;
        }
    }

    return len;
}

static int sendData(char *rsp, int rsplen){
    int sn = 2;
    int nRd = 0;
    MSG_HEAD_T* msg_head_ptr = NULL;
    ETB_DUMP_HEAD* lpHeader = NULL;
    ETB_DUMP_DATA* lpData = NULL;
    FILE *fp = NULL;

    ENG_LOG("%s: ", __FUNCTION__);

    msg_head_ptr = (MSG_HEAD_T*)(rsp+1);
    lpHeader = (ETB_DUMP_HEAD*)(msg_head_ptr+1);
    lpData = (ETB_DUMP_DATA*)(lpHeader+1);

    fp = fopen(DEV_ETB_PATH, "r");
    if (fp == NULL){
        ENG_LOG("open %s fail", DEV_ETB_PATH);
        return 0;
    }

    while ( (nRd = fread(&(lpData->data), sizeof(char), ETB_DUMP_BUFF, fp)) > 0){
        ENG_LOG("etb read return %d", nRd);
        //nRd = translate_packet(&(lpData->data), nRd);
        //ENG_LOG("translate return %d", nRd);
        msg_head_ptr->seq_num = sn++;
        msg_head_ptr->len = sizeof(MSG_HEAD_T)+sizeof(ETB_DUMP_HEAD)+sizeof(lpData->type)+nRd;
        lpHeader->cmd = ETB_DUMP_CMD;
        lpHeader->len = sizeof(lpData->type)+nRd;
        lpData->type = ETB_DUMP_DUMPING;
        rsp[msg_head_ptr->len+1] = 0x7E;
        if (g_func[WRITE_TO_HOST_DIAG] != NULL){
            g_func[WRITE_TO_HOST_DIAG](rsp, msg_head_ptr->len+2);
        }
    };

    fclose(fp);

    return sn;
}

static int etb_handler(char *buff, int len, char *rsp, int rsplen)
{
    int ret = 0;
    MSG_HEAD_T* msg_head_ptr = NULL;
    ETB_DUMP_HEAD* lpHeader = NULL;
    ETB_DUMP_DATA* lpData = NULL;
    int sn = 0;

    if(NULL == buff || NULL == rsp){
        ENG_LOG("%s,null pointer",__FUNCTION__);
        return 0;
    }

    memcpy(rsp, buff, sizeof(MSG_HEAD_T)+1);
    msg_head_ptr = (MSG_HEAD_T*)(rsp+1);
    lpHeader = (ETB_DUMP_HEAD*)(msg_head_ptr+1);
    lpData = (ETB_DUMP_DATA*)(lpHeader+1);

    //read begin
    msg_head_ptr->len = sizeof(MSG_HEAD_T)+sizeof(ETB_DUMP_HEAD)+sizeof(lpData->type);
    msg_head_ptr->seq_num = 1;
    lpHeader->cmd = ETB_DUMP_CMD;
    lpHeader->len = sizeof(lpData->type);
    lpData->type = ETB_DUMP_START;
    rsp[msg_head_ptr->len+1] = 0x7E;
    if (g_func[WRITE_TO_HOST_DIAG] != NULL){
        g_func[WRITE_TO_HOST_DIAG](rsp, msg_head_ptr->len+2);
    }

    //send data
    sn = sendData(rsp, rsplen);

    //read end
    msg_head_ptr->len = sizeof(MSG_HEAD_T)+sizeof(ETB_DUMP_HEAD)+sizeof(lpData->type);
    msg_head_ptr->seq_num = sn;
    lpHeader->cmd = ETB_DUMP_CMD;
    lpHeader->len = sizeof(lpData->type);
    lpData->type = ETB_DUMP_END;
    rsp[msg_head_ptr->len+1] = 0x7E;
    if (g_func[WRITE_TO_HOST_DIAG] != NULL){
        g_func[WRITE_TO_HOST_DIAG](rsp, msg_head_ptr->len+2);
    }

    return ENG_DIAG_NO_RESPONSE;
}

void register_this_module_ext(struct eng_callback *reg, int *num)
{
    int moudles_num = 0;
    ENG_LOG("register_this_module_ext :libetb");

    (reg+moudles_num)->type = 0x62;
    (reg+moudles_num)->subtype = 0x00;
    (reg+moudles_num)->diag_ap_cmd = 0x33;
    (reg+moudles_num)->eng_diag_func = etb_handler; // rsp function ptr
    (reg+moudles_num)->eng_set_writeinterface_func = get_fw_ptr;
    moudles_num++;

    *num = moudles_num;
    ENG_LOG("register_this_module_ext: %d - %d",*num, moudles_num);
}
