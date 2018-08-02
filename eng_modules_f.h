#ifndef __ENG_DIAG_MODULES_H__F
#define __ENG_DIAG_MODULES_H__F

#include "eng_list.h"
#include "eng_modules.h"

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

/*struct eng_callback{
    unsigned int diag_ap_cmd; //data area: unsigned int for data command
    unsigned char type; //command
    unsigned char subtype; //data command
    char at_cmd[32];
    int also_need_to_cp;
    int (*eng_diag_func)(char *buf, int len, char *rsp, int rsplen);
    int (*eng_linuxcmd_func)(char *req, char *rsp);
};
*/

struct eng_callback_func{
    char moudel_des[128];
    void(*eng_cb)(struct fw_callback *reg);
};

typedef int (*QUERYINTERFACE)(char *module_desc, void** ptrFunc);

struct fw_callback{
    QUERYINTERFACE ptfQueryInterface;
};


/*typedef struct eng_modules_info
{
    struct  list_head node;
    struct  eng_callback callback;
}eng_modules;
*/

typedef struct eng_callbacks_info
{
    struct  list_head node;
    struct  eng_callback_func callback;
}eng_callbacks;

//int eng_modules_load(struct list_head *head);

struct eng_fw_des_ptrFunc{
    char func_des[128];
    int  ptrFunc;
};

typedef struct eng_fw_ptrFunc_info
{
    struct list_head node;
    struct eng_fw_des_ptrFunc fw_ptrFunc;
}eng_fw_ptrFunc;

#endif
