#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <cutils/log.h>

#include "chnmgr.h"
#include "calibration.h"

int chnl_diag_send(char *buf, int len, char *rsp, int rsp_len){
    struct list_head* pHead;
    eng_modules *modules_list = NULL;
    struct list_head *list_find;
    int rlen = -1;
    MSG_HEAD_T *msg_head_ptr = (MSG_HEAD_T *)(buf + 1);
    unsigned int *data_cmd = NULL;
    TOOLS_DIAG_AP_CMD_T *apcmd = NULL;
    byte *data = NULL;

    pHead = chnl_get_call_list_head();
    if (pHead == NULL)
    {
        return -1;
    }

    list_for_each(list_find, pHead)
    {
        modules_list = list_entry(list_find, eng_modules, node);
	 ENG_LOG("%s data cmd id:type=%d,subtype=%d,ap_cmd=%d", __FUNCTION__,modules_list->callback.type ,modules_list->callback.subtype,modules_list->callback.diag_ap_cmd);
        if (msg_head_ptr->type == modules_list->callback.type && msg_head_ptr->subtype == modules_list->callback.subtype) {
	      // diag command: type(unsigned char) + sub_type(unsigned char) + data_cmd(unsigned int)
	      if (0x5D == msg_head_ptr->type){
	        data_cmd = (unsigned int *)(buf + 1 + sizeof(MSG_HEAD_T));
	        if (*data_cmd != modules_list->callback.diag_ap_cmd) {
	          ENG_LOG("%s data cmd is not matched!", __FUNCTION__);
	          return -1;
	        }
	      }

	      // diag command: type(unsigned char) + sub_type(unsigned char) + TOOLS_DIAG_AP_CMD_T
	      if (0x62 == msg_head_ptr->type){//DIAG_CMD_APCALI
	        apcmd = (TOOLS_DIAG_AP_CMD_T *)(buf + 1 + sizeof(MSG_HEAD_T));
	        if (apcmd->cmd != modules_list->callback.diag_ap_cmd) {
	          ENG_LOG("%s apcmd->cmd is not matched!", __FUNCTION__);
	          return -1;
	        }
	      }

	      // diag command: type(unsigned char) + sub_type(unsigned char) + byte, for autotest test case which have same type+subtype
	      if (0x38 == msg_head_ptr->type){
	        data = (byte *)(buf + 1 + sizeof(MSG_HEAD_T));
		 ENG_LOG("chnl_at_send (int)(*data) =%d", (int)(*data) );
	        if (modules_list->callback.diag_ap_cmd != -1 && (int)(*data) != modules_list->callback.diag_ap_cmd)  {
	          ENG_LOG("%s data is not matched!", __FUNCTION__);
	          return -1;
	        }
	      }

	      if (NULL != modules_list->callback.eng_diag_func) {
	        rlen = modules_list->callback.eng_diag_func(buf, len, rsp, rsp_len/2);
	        if (rlen == ENG_DIAG_RET_UNSUPPORT){
	            ENG_LOG("%s eng_diag_func return %d, continue find next callback func", __FUNCTION__, ENG_DIAG_RET_UNSUPPORT);
	            return -1;
	        }
	      } else {
	        ENG_LOG("%s: Dymic eng_diag_func == NULL\n",__FUNCTION__);
	        return -1;
	      }
    }
    }
    ENG_LOG("chnl_at_send ret=%d", rlen);
    return rlen;
}

