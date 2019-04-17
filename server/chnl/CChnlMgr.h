#ifndef __CHNL_MGR_H__
#define __CHNL_MGR_H__

#include "CChnl.h"
#include "CChnlThreadMgr.h"
#include "CBase.h"
#include "../module/CModuleMgr.h"

class CChnlMgr:public CBase{
    public:
        virtual ~CChnlMgr();

        static CChnlMgr* getInstance(CChnlThreadMgr* tMgr, CDevMgr* dMgr, CModuleMgr* mMgr){
            return new CChnlMgr(tMgr, dMgr, mMgr);
        }

        const char* TAG(){return "CChnlMgr";}

        int load(char* dir);
        CChnl* find(const char* name);

        void run(const char* bootmode);
        void resetWithChnlName(const char* chnlname);
        void resetWithDevName(const char* devname);
        void resetWithPortName(const char* devname, const char* portname);

        bool enable(const char *bootmode, bool enable);

        void print();

    private:
        EngVector<CChnl*> m_chnlList;
        CChnlThreadMgr* m_lpChnlThreadMgr;
        CDevMgr* m_lpChnlDevMgr;
        CModuleMgr* m_lpModMgr;

        CChnlMgr(CChnlThreadMgr* tMgr, CDevMgr* dMgr, CModuleMgr* mMgr);

        int addChnl(CChnl* dev);
};

#endif