#ifndef __ENG_DIAG_MODULES_H__
#define __ENG_DIAG_MODULES_H__

#include "eng_list.h"

#ifndef byte
typedef unsigned char  byte;
#endif

#ifndef uchar 
typedef unsigned char  uchar;
#endif

#ifndef uint
typedef unsigned int   uint;
#endif // uint

#ifndef ushort
typedef unsigned short ushort;
#endif

typedef int (*DYMIC_WRITETOPC_FUNC)(char *rsp, int len);

struct eng_callback{
    unsigned int diag_ap_cmd; //data area: unsigned int for data command
    unsigned char type; //command
    unsigned char subtype; //data command
    char at_cmd[32];
    int also_need_to_cp;
    int (*eng_diag_func)(char *buf, int len, char *rsp, int rsplen);
    int (*eng_linuxcmd_func)(char *req, char *rsp);
    int (*eng_set_writeinterface_func)(DYMIC_WRITETOPC_FUNC * write_interface_ptr);
};


typedef struct eng_modules_info
{
    struct  list_head node;
    struct  eng_callback callback;
}eng_modules;

typedef enum {
  WRITE_TO_START = 0,
  WRITE_TO_HOST_DIAG = WRITE_TO_START,
  WRITE_TO_HOST_LOG,
  WRITE_TO_HOST_AT,
  WRITE_TO_MODEM_DIAG,
  WRITE_TO_MODEM_LOG,
  WRITE_TO_MODEM_AT,
  WRITE_TO_MAX = WRITE_TO_START + 16,
} WRITE_INTERFACE_E;

int eng_modules_load(struct list_head *head);

#endif
