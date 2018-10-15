#include "engpc_cmd_server.h"

//support adb cmd
int handle_cmd(char *buff, char *rsp)
{
    int ret = -1;
    int is_diag_cmd = 0;
    char *ptr;
    ENG_LOG("%s: buff = %s", __FUNCTION__ , buff);
    //do some work
     if(buff[0] == 0x7e)
    {
        ptr = buff + 1 + sizeof(MSG_HEAD_T);
        is_diag_cmd = 1;
    }
    else
    {
        ptr = strdup(buff) ;
    }
    ENG_LOG("%s: ptr = %s,is_diag_cmd=%d", __FUNCTION__ , ptr,is_diag_cmd);
    if(is_diag_cmd  == 0){
	ret = chnl_send(CHNL_AT, ptr, strlen(ptr), rsp, strlen(rsp));
    }else{
	ret = chnl_send(CHNL_DIAG, ptr, strlen(ptr), rsp, strlen(rsp));
    }
	
    ENG_LOG("%s: rsp = %s", __FUNCTION__ , rsp);

    ENG_LOG("%s: ret = %d", __FUNCTION__ , ret);
    return ret;
}

void *cmd_server_thread(void *)
{
	int cmd_server_fd= 0;
    int client_fd = 0;
    int connect_fd = -1;
    static char recv_buf[MAX_BUF_LEN];
    static char data_buff[MAX_BUF_LEN];
    static char rsp_buff[MAX_BUF_LEN] = "Result";
    int nRead = 0, nWrite = 0, nWriteOffset = 0;;
    
    if(create_socket(&SOCKET_SERVER_FD, SOCKET_SERVER_NAME)){
		ENG_LOG("create_socket : fd=%d ,name=%s failed!", SOCKET_SERVER_FD, SOCKET_SERVER_NAME);
    }
    ENG_LOG("create_socket done! fd=%d ,name=%s", SOCKET_SERVER_FD, SOCKET_SERVER_NAME);
    if (cmd_server_fd < 0) {
        ENG_LOG("%s: cannot create local socket server\n", __FUNCTION__);
	 return NULL;
    }
	  
    ENG_LOG("%s: accept client_fd =%d\n", __FUNCTION__, client_fd);
    while(1) {
        if ((connect_fd = accept_client(SOCKET_SERVER_FD)) < 0)
        {
            ENG_LOG("accept socket failed!");
            ENG_LOG("accept socket error:%s",strerror(errno));
            if (cmd_server_fd >= 0) close(cmd_server_fd);
            return NULL;
        }
        
        //read from client
        ENG_LOG("read....\n");
        int read_error = 0;
        int iRet = 0;
        memset(recv_buf, '\0', sizeof(recv_buf));
        if( (iRet = read(connect_fd, recv_buf, sizeof(recv_buf))) <= 0)
        {
            ENG_LOG("recv cmd error:%s",strerror(errno));
        }
        /*while(1){
            int iRet = 0;
            memset(recv_buf, '\0', sizeof(recv_buf));
            if( (iRet = read(connect_fd, recv_buf, sizeof(recv_buf))) <= 0)
            {
                ENG_LOG("recv cmd error:%s",strerror(errno));
                read_error = -1;
                break;
            }
            ENG_LOG("read return %d", iRet);
            memcpy(data_buff+nRead, recv_buf, iRet);
            nRead += iRet;
            if (nRead == MAX_BUF_LEN) {
                break;
            }
        }*/
        ENG_LOG("read return %d", iRet);
        memcpy(data_buff, recv_buf, iRet);
        ENG_LOG("read return recv_buf=%s \n data_buff=%s", recv_buf, data_buff);
        ENG_LOG("read succ...\n");
        
        //handle read buf
        handle_cmd(recv_buf , rsp_buff);
        
        //write result to client
        ENG_LOG("write....\n");
        //nWrite = 0;
        iRet = 0;
        do {
            iRet = write(connect_fd, rsp_buff, strlen(rsp_buff));
        }while(iRet < 0 && ((errno == EINTR) || (errno == EAGAIN)));
        /*while(nWrite < MAX_BUF_LEN && read_error >= 0){
            int iRet = 0;
            do {
                iRet = write(connect_fd, rsp_buff+nWrite, MAX_BUF_LEN-nWrite);
            }while(iRet < 0 && ((errno == EINTR) || (errno == EAGAIN)));

            if (iRet >= 0) {
                nWrite += iRet;
            } else {
                break;
            }
        }*/
        ENG_LOG("write rsp_buff=%s", rsp_buff);
        ENG_LOG("write succ....\n");
        close(connect_fd);
        
    }
    close(cmd_server_fd);
    return NULL;
}

int cmd_server_init(void)
{
    ENG_LOG("cmd_server_init start");
    pthread_t t;
    pthread_create(&t, NULL, cmd_server_thread, NULL);
    ENG_LOG("cmd_server_init end!");
    return 0;
}
