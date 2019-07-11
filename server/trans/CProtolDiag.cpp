#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "CProtolDiag.h"
#include "modules.h"

CProtolDiag::CProtolDiag(){
    m_nDiagAT = 0;
    m_bPending = false;
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
            int nPendingLen = 0;
            if (m_bPending) nPendingLen = strlen(PENDING_MARK);

            msg_head_ptr.seq_num = 0;
            msg_head_ptr.type = 0x9c;
            msg_head_ptr.subtype = 0x00;
            len = strlen(buff) + sizeof(MSG_HEAD_T)-nPendingLen;
            memmove(buff+sizeof(MSG_HEAD_T)+1, buff, strlen(buff)-nPendingLen);
            memcpy(buff+1, &msg_head_ptr, sizeof(MSG_HEAD_T));
            ((MSG_HEAD_T *)(buff+1))->len = len;
            buff[0] = 0x7E;
            buff[len+1] = 0x7E;
            len += 2;
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

bool CProtolDiag::checkPending(char*rsp, int nlen){
    int ret = 0;

    if (m_nDiagAT){
        int at_ret_error = ((strcasestr(rsp, "ERROR")) != NULL) ? 1 : 0;

        // whether rsp contains "\r\nPENDING\r\n"
        if (!at_ret_error && (strcasestr(rsp, PENDING_MARK)) != NULL) {
            int pending_mark_len = strlen(PENDING_MARK);
            if (0 == strncmp(rsp + strlen(rsp) - pending_mark_len, PENDING_MARK, pending_mark_len)) {
                return 1;
            }
        }
    }

    return 0;
}
