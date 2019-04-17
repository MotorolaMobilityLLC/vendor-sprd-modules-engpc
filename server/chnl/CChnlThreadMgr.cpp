#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include "eng_vector.h"

#include "CChnlThreadMgr.h"
#include "CChnlThread.h"

CChnlThreadMgr::CChnlThreadMgr(){

}

CChnlThreadMgr::~CChnlThreadMgr(){
}

void CChnlThreadMgr::add(CChnlThread* lpThread){
    m_threadList.push_back(lpThread);
}
