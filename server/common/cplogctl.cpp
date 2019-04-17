#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <cutils/sockets.h>
#include <pthread.h>

#include "cplogctl.h"
#include "englog.h"

CCPLogCtl::CCPLogCtl(){
    m_lpChnlMgr = NULL;
    m_fdSockSrv = -1;

    memset(m_socket_rbuf, 0, sizeof(m_socket_rbuf));
    memset(m_socket_wbuf, 0, sizeof(m_socket_wbuf));
}

CCPLogCtl::~CCPLogCtl(){
}

void CCPLogCtl::attach(CChnlMgr* lpChnlMgr){
    
}

void CCPLogCtl::run(){
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    pthread_create(&m_idThread, &attr, threadMain, (void*)this);
}

void* CCPLogCtl::threadMain(void *arg){
    CCPLogCtl* lpCpLogCtl = (CCPLogCtl*)arg;
    int sockfd = -1;

    /* creat socket server */
    if (-1 == create_socket_local_server(lpCpLogCtl)) {
        EngLog::error("eng_soc thread start error");
        return NULL;
    }

    while (1) {
        EngLog::info("%s eng_connect_fd=%d", __FUNCTION__, sockfd);
        if ((sockfd = ::accept(lpCpLogCtl->m_fdSockSrv, (struct sockaddr *)NULL, NULL)) == -1) {
            EngLog::error("accept socket error: %s(errno: %d)", strerror(errno), errno);
            continue;
        }

        EngLog::info("%s connected! eng_connect_fd:%d socket_name=%s", __FUNCTION__, sockfd, SOCK_CPLOGCTL_NAME);
        eng_socket_rw_thread(&sockfd, (void* )lpCpLogCtl);
    }

    out:
        ::close(lpCpLogCtl->m_fdSockSrv);
        lpCpLogCtl->m_fdSockSrv = -1;

    return NULL;
}

int CCPLogCtl::create_socket_local_server(CCPLogCtl* lpCpLogCtl) {
    lpCpLogCtl->m_fdSockSrv = socket_local_server(SOCK_CPLOGCTL_NAME, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if (lpCpLogCtl->m_fdSockSrv < 0) {
        EngLog::error("open %s failed: %s\n", SOCK_CPLOGCTL_NAME, strerror(errno));
        return -1;
    }
    if (::listen(lpCpLogCtl->m_fdSockSrv, 4) < 0) {
        EngLog::error("listen %s failed: %s\n", SOCK_CPLOGCTL_NAME, strerror(errno));
        ::close(lpCpLogCtl->m_fdSockSrv);
        return -1;
    }
    return 0;
}

void CCPLogCtl::eng_socket_rw_thread(void *fd, void *arg)
{
    int soc_fd;
    int ret;
    int length;
    int type;
    fd_set readfds;
    char buf[2*ENG_DATA_LENGTH] = {0};
    ENG_DATA_T *eng_data_ptr = NULL;
    CCPLogCtl* lpCpLogCtl = (CCPLogCtl*)arg;

    soc_fd = *(int *)fd;
    EngLog::info("%s soc_fd = %d ", __FUNCTION__, soc_fd);
    while(1) {
        FD_ZERO(&readfds);
        FD_SET(soc_fd,&readfds);
        ret = ::select(soc_fd+1,&readfds,NULL,NULL,NULL);
        if (ret < 0) {
            EngLog::error("%s  ret = %d, break",__FUNCTION__, ret);
            break;
        }
        memset(lpCpLogCtl->m_socket_rbuf,0,ENG_DATA_LENGTH);
        memset(lpCpLogCtl->m_socket_wbuf,0,ENG_DATA_LENGTH);
        if (FD_ISSET(soc_fd,&readfds)) {
            length = ::read(soc_fd,lpCpLogCtl->m_socket_rbuf,ENG_DATA_LENGTH);
            if (length <= 0) {
                EngLog::error("%s length = %d, break",__FUNCTION__, length);
                break;
            }
            EngLog::info("%s  socket_read_buf = %d %d %d",__FUNCTION__, lpCpLogCtl->m_socket_rbuf[0], lpCpLogCtl->m_socket_rbuf[1], lpCpLogCtl->m_socket_rbuf[2]);
            eng_data_ptr = (ENG_DATA_T *)(lpCpLogCtl->m_socket_rbuf);
            //eng_data_ptr->result = cplogctrl_setlocation(eng_data_ptr->log_type + '0', eng_data_ptr->location + '0', 1);
            memcpy(lpCpLogCtl->m_socket_wbuf,(char *)eng_data_ptr,ENG_DATA_LENGTH);
            EngLog::info("%s  socket_wbuf = %d %d %d",__FUNCTION__, lpCpLogCtl->m_socket_wbuf[0], lpCpLogCtl->m_socket_wbuf[1], lpCpLogCtl->m_socket_wbuf[2]);
            ::write(soc_fd,lpCpLogCtl->m_socket_wbuf,ENG_DATA_LENGTH);
            EngLog::info("%s -2-eng_connect_fd=%d", __FUNCTION__, soc_fd);
        }
        EngLog::info("%s -3-eng_connect_fd=%d", __FUNCTION__, soc_fd);
    }
    EngLog::info("%s CLOSE_SOCKET",__FUNCTION__);
    ::close(soc_fd);
}