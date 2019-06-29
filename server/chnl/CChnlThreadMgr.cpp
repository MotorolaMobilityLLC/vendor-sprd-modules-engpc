#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include "eng_vector.h"

#include "CChnlThreadMgr.h"
#include "CChnlThread.h"
#include "CPort.h"

#define stricmp strcasecmp

CChnlThreadMgr::CChnlThreadMgr(){

}

CChnlThreadMgr::~CChnlThreadMgr(){
}

void CChnlThreadMgr::add(CChnlThread* lpThread){
    m_threadList.push_back(lpThread);
}

void CChnlThreadMgr::restartWithDev(const char* name){
    for(int i = 0; i < m_threadList.size(); i++){
        CPort* lpPort = m_threadList[i]->getSrcPort();
        if(lpPort != NULL && stricmp(lpPort->getDevName(), name) == 0){
            CChnlThread* lpThread = m_threadList[i];
            if (!lpThread->isRunning()){
                lpThread->run();
            }else{
                lpThread->stop();
            }
        }
    }
}