#ifndef __ENGPC_CPLOGCTL_H__
#define __ENGPC_CPLOGCTL_H__

#include "CChnlMgr.h"

#define SOCK_CPLOGCTL_NAME "engpc_soc.l"
#define ENG_DATA_LENGTH 3

typedef struct{
    char sn;
    char log_type;
    union{
        char location;
        char result;
    };
}ENG_DATA_T;

class CCPLogCtl {
    public:
        pthread_t m_idThread;
        int m_fdSockSrv;

        char m_socket_rbuf[ENG_DATA_LENGTH];
        char m_socket_wbuf[ENG_DATA_LENGTH];


        CCPLogCtl();
        ~CCPLogCtl();

        void attach(CChnlMgr* lpChnlMgr);
        void run();

    private:
        CChnlMgr* m_lpChnlMgr;

        static int create_socket_local_server(CCPLogCtl* lpCpLogCtl);
        static void* threadMain(void *arg);
        static void eng_socket_rw_thread(void *fd, void *arg);
};


#endif