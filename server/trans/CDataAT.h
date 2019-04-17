#ifndef __CDATA_AT_H__
#define __CDATA_AT_H__

#include "CProtol.h"
#include "../module/CModuleMgr.h"

class CDataAT:public CProtol{
    public:
        CDataAT(CModuleMgr *lpModMgr);
        virtual ~CDataAT();

        virtual int process(char* req, int reqlen, char* rsp, int rsplen, int& retlen);

    private:
        CModuleMgr* m_lpModMgr;

};

#endif