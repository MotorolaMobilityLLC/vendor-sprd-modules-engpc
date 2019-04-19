#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "CProtolDiag.h"
#include "modules.h"

CProtolDiag::CProtolDiag(){
    m_nDiagAT = 0;
}

CProtolDiag::~CProtolDiag(){
}

int CProtolDiag::decode(char* buff, int len){
    int i, j, retlen;
    unsigned char tmp;

    info("%s: len=%d", __FUNCTION__, len);

    retlen = len;
    for (i = 1; i < len-2; i++) {
        if ((buff[i] == 0x7d) || (buff[i] == 0x7e)) {
            tmp = buff[i + 1] ^ 0x20;
            buff[i] = tmp;
            for(j = i + 1; j < len; j ++){
                buff[j] = buff[j+1];
            }
            len--;
            retlen--;
        }
    
    }

    info("%s: retlen=%d", __FUNCTION__, retlen);

    m_nDiagAT = (buff[7] == 0x68)?1:0;

    return retlen;
}

int CProtolDiag::encode(char* buff, int len){
    int i, j, retlen;
    char tmp;

    info("%s: len=%d", __FUNCTION__, len);

    if ((m_retType == DYMIC_RET_DEAL_SUCCESS || m_retType == DYMIC_RET_ALSO_NEED_TO_CP) && m_nDiagAT){
        do {
            MSG_HEAD_T msg_head_ptr = {0};
            msg_head_ptr.seq_num = 0;
            msg_head_ptr.type = 0x9c;
            msg_head_ptr.subtype = 0x00;
            len = strlen(buff) + sizeof(MSG_HEAD_T);
            memmove(buff+sizeof(MSG_HEAD_T), buff, strlen(buff));
            memcpy(buff, &msg_head_ptr, sizeof(MSG_HEAD_T));
            ((MSG_HEAD_T *)buff)->len = len;
        } while (0);

        info("%s: new len=%d", __FUNCTION__, len);
    }

    retlen = len;
    for (i = 1; i < len-2; i++) {
        if ((buff[i] == 0x7d) || (buff[i] == 0x7e)) {
            tmp = buff[i] ^ 0x20;
            info("%s: tmp=%x, buf[%d]=%x", __FUNCTION__, tmp, i, buff[i]);
            buff[i] = 0x7d;
            for (j = len; j > i + 1; j--) {
                buff[j] = buff[j - 1];
            }
            buff[i + 1] = tmp;
            len++;
            retlen++;
        }
    }

    info("%s: retlen=%d", __FUNCTION__, retlen);
    return retlen;
}

FRAME_TYPE CProtolDiag::checkframe(char* buff, int nlen){
    FRAME_TYPE ret = FRAME_INVALID;
    if (buff[0] == 0x7E && buff[nlen-1] == 0x7E){
        ret = FRAME_COMPLETE;
    }

    return ret;
}

int CProtolDiag::findframe(char* buff, int nlen){
    return nlen;
}
