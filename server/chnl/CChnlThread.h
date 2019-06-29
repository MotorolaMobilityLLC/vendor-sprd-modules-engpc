#ifndef __CHNL__THREAD__H__
#define __CHNL__THREAD__H__

#include <pthread.h>

#include "channel.h"
#include "CDevMgr.h"
#include "../module/CModuleMgr.h"
#include "CPort.h"

class CChnlThread{
    public:
        CChnlThread(CModuleMgr* mMgr, char* name, CPort* lpSrc, CPort* lpDst, int dataType, int apProcess);
        ~CChnlThread();

        CPort* getSrcPort();
        CPort* getDstPort();
        int getDataType();
        int getApProcess();
        int getThreadId();
        char* getName();
        CModuleMgr* getModMgr() {return m_lpModMgr;}

        bool run();
        bool stop();
        bool isRunning();

    private:
        char m_name[CHNL_NAME_LEN];
        CModuleMgr* m_lpModMgr;
        CPort* m_lpPortSrc;
        CPort* m_lpPortDst;
        int m_dataType;
        int m_apProcess;
        pthread_t m_idThread;
        bool m_isRunning;

        static void* threadMain(void *);
};

#endif