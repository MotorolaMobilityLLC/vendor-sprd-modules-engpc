#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>


#include "CTrans.h"

#include "channel.h"
#include "dev.h"
#include "CChnlThread.h"
#include "CDataDiag.h"
#include "CDataAT.h"
#include "CDataLog.h"
#include "CDataAgdsp.h"


CChnlThread::CChnlThread(CModuleMgr* mMgr, char* name, CPort* lpSrc, CPort* lpDst, int dataType, int apProcess){
    m_lpModMgr = mMgr;
    m_lpPortSrc = lpSrc;
    m_lpPortDst = lpDst;
    m_dataType = dataType;
    m_apProcess = apProcess;
    strcpy(m_name, name);
}

CChnlThread::~CChnlThread(){
}

CPort* CChnlThread::getSrcPort(){
    return m_lpPortSrc;
}

CPort* CChnlThread::getDstPort(){
    return m_lpPortDst;
}

int CChnlThread::getDataType(){
    return m_dataType;
}

int CChnlThread::getApProcess(){
    return m_apProcess;
}

int CChnlThread::getThreadId(){
    return m_idThread;
}

char* CChnlThread::getName(){
    return m_name;
}

bool CChnlThread::run(){
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  
  return pthread_create(&m_idThread, &attr, threadMain, (void*)this);
}

void* CChnlThread::threadMain(void *arg){
    CChnlThread* lpThread = (CChnlThread*)arg;
    CTrans* lpTrans = NULL;

    if (lpThread->m_dataType == DATA_DIAG || lpThread->m_dataType == DATA_WCN_DIAG){
        lpTrans = new CDataDiag(lpThread->getModMgr());
    }else if(lpThread->m_dataType == DATA_AT||lpThread->m_dataType == DATA_WCN_AT){
        lpTrans = new CDataAT(lpThread->getModMgr());
    }else if(lpThread->m_dataType == DATA_LOG){
        lpTrans = new CDataLog(lpThread->getModMgr());
    }else if(lpThread->m_dataType == DATA_AGDSP_LOG || lpThread->m_dataType == DATA_AGDSP_PCM || lpThread->m_dataType == DATA_AGDSP_MEM){
        lpTrans = new CDataAgdsp(lpThread->getModMgr());
    }else{
        return NULL;
    }

    lpTrans->init(lpThread->m_name, lpThread->m_lpPortSrc, lpThread->m_lpPortDst, lpThread->m_dataType, lpThread->m_apProcess);
    while(-1 == lpTrans->trans()){
        break;
    }

    return NULL;
}
