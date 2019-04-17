#ifndef __CHNL__THREAD__MGR__H__
#define __CHNL__THREAD__MGR__H__

#include "eng_vector.h"

#include "CChnlThread.h"

class CChnlThreadMgr{
    public:
        ~CChnlThreadMgr();

        static CChnlThreadMgr* getInstance(){
            return new CChnlThreadMgr();
        }

        void add(CChnlThread* lpThread);
    private:
        EngVector<CChnlThread* > m_threadList;
        CChnlThreadMgr();
};

#endif