// 
// Spreadtrum Auto Tester
//
// anli   2012-11-09
//
#ifndef _AUTOTEST_20121109_H__
#define _AUTOTEST_20121109_H__

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define  LOG_TAG "BBAT"
#include <cutils/log.h>

#include <assert.h>
#include <errno.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define LOGI ALOGI
#define LOGD ALOGD
#define LOGW ALOGW
#define LOGE ALOGE
#define FUN_ENTER             LOGD("[ %s ++ ]\n", __FUNCTION__)
#define FUN_EXIT              LOGD("[ %s -- ]\n", __FUNCTION__)

struct eng_callback{
    unsigned int diag_ap_cmd; //data area: unsigned int for data command
    unsigned char type; //command
    unsigned char subtype; //data command
    char at_cmd[32];
    int (*eng_diag_func)(char *buf, int len, char *rsp, int rsplen);
    int (*eng_linuxcmd_func)(char *req, char *rsp);
};

// This is the communication frame head
typedef struct msg_head_tag {
  unsigned int seq_num;  // Message sequence number, used for flow control
  unsigned short len;    // The totoal size of the packet "sizeof(MSG_HEAD_T)
  // + packet size"
  unsigned char type;     // Main command type
  unsigned char subtype;  // Sub command type
} __attribute__((packed)) MSG_HEAD_T;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DEBUG_20121109_H__
