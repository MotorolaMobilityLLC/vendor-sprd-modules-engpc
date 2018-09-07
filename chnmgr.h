#ifndef _ENFCHNMGR_H_
#define _ENFCHNMGR_H_

#include "eng_modules_f.h"
#include "eng_diag.h"
#include "eng_modules.h"
#include "engopt.h"

typedef enum
{
    CHNL_DIAG,
    CHNL_AT,
    CHNL_NONE,
}CHNL_TYPE;

struct list_head* chnl_get_call_list_head();
int chnl_send(CHNL_TYPE type, char *buf, int len, char *rsp, int rsp_len);
int chnl_at_send(char *buf, int len, char *rsp, int rsp_len);
int chnl_diag_send(char *buf, int len, char *rsp, int rsp_len);
int chnl_send_at_interface(char *buf, int len);
int chnl_send_diag_interface(char *buf, int len);

#endif