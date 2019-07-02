#ifndef __CPROTOL_DIAG_H__
#define __CPROTOL_DIAG_H__

#include "CProtol.h"

class CProtolDiag:public CProtol{
    public:
        CProtolDiag();
        virtual ~CProtolDiag();

        virtual int decode(char* buff, int nlen);
        virtual int encode(char* buff, int nlen);
        virtual FRAME_TYPE checkframe(char* buff, int nlen);
        virtual int findframe(char* buff, int nlen);

    public:
        int m_nDiagAT;
};

#endif
