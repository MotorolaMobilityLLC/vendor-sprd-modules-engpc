#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <termios.h>
#include <semaphore.h>
#include <cutils/sockets.h>
#include "vlog.h"
#include "eng_socket.h"
#include "engopt.h"
#include "eng_pcclient.h"

#define ENG_DATA_LENGTH 3

static int eng_command_server;

static char socket_rbuf[ENG_DATA_LENGTH] = {0};
static char socket_wbuf[ENG_DATA_LENGTH] = {0};

static int eng_connect_fd = -1;
eng_thread_t t_eng_soc_rw;

static int create_socket_local_server(int *fd, char *file) {
    *fd = socket_local_server(file, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if (*fd < 0) {
        ENG_LOG("open %s failed: %s\n", file, strerror(errno));
        return -1;
    }
    if (listen(*fd, 4) < 0) {
        ENG_LOG("listen %s failed: %s\n", file, strerror(errno));
        close(*fd);
        return -1;
    }
    return 0;
}

static int connect_socket_local_server(int *fd, char *name) {
    *fd = socket_local_client(name, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if (*fd < 0) {
        ENG_LOG("open %s failed: %s\n", name, strerror(errno));
        return -1;
    }
    return 0;
}


static void eng_socket_rw_thread(void *fd)
{
    int soc_fd;
    int ret;
    int length;
    int type;
    fd_set readfds;
    char buf[2*ENG_DATA_LENGTH] = {0};
    ENG_DATA_T *eng_data_ptr = NULL;

    soc_fd = *(int *)fd;
    ENG_LOG("%s soc_fd = %d eng_connect_fd=%d", __FUNCTION__, soc_fd, eng_connect_fd);
    while(1) {
        FD_ZERO(&readfds);
        FD_SET(soc_fd,&readfds);
        ret = select(soc_fd+1,&readfds,NULL,NULL,NULL);
        if (ret < 0) {
            ENG_LOG("%s  ret = %d, break",__FUNCTION__, ret);
            break;
        }
        memset(socket_rbuf,0,ENG_DATA_LENGTH);
        memset(socket_wbuf,0,ENG_DATA_LENGTH);
        if (FD_ISSET(soc_fd,&readfds)) {
            length = read(soc_fd,socket_rbuf,ENG_DATA_LENGTH);
            if (length <= 0) {
                ENG_LOG("%s length = %d, break",__FUNCTION__, length);
                break;
            }
            ENG_LOG("%s  socket_read_buf = %d %d %d",__FUNCTION__, socket_rbuf[0], socket_rbuf[1], socket_rbuf[2]);
            eng_data_ptr = (ENG_DATA_T *)socket_rbuf;
            eng_data_ptr->result = cplogctrl_setlocation(eng_data_ptr->log_type + '0', eng_data_ptr->location + '0');
            memcpy(socket_wbuf,(char *)eng_data_ptr,ENG_DATA_LENGTH);
            ENG_LOG("%s  socket_wbuf = %d %d %d",__FUNCTION__, socket_wbuf[0], socket_wbuf[1], socket_wbuf[2]);
            write(soc_fd,socket_wbuf,ENG_DATA_LENGTH);
            ENG_LOG("%s -2-eng_connect_fd=%d", __FUNCTION__, eng_connect_fd);
        }
        ENG_LOG("%s -3-eng_connect_fd=%d", __FUNCTION__, eng_connect_fd);
    }
    ENG_LOG("%s CLOSE_SOCKET",__FUNCTION__);
    close(soc_fd);
}

void *eng_socket_thread(char *run_type) {
  char socket_name[32] = {0};

  //0 != strcmp(run_type, "wcn")
  memset(socket_name, 0, sizeof(socket_name));
  sprintf(socket_name, "%s%s", "engpc_soc.", run_type);

  /* creat socket server */
  if (-1 == create_socket_local_server(&eng_command_server, socket_name)) {
    ENG_LOG("eng_soc thread start error");
    return NULL;
  }

  while (1) {
    ENG_LOG("%s eng_connect_fd=%d", __FUNCTION__, eng_connect_fd);
    if ((eng_connect_fd = accept(eng_command_server, (struct sockaddr *)NULL, NULL)) == -1) {
      ENG_LOG("accept socket error: %s(errno: %d)", strerror(errno), errno);
      continue;
    }

    ENG_LOG("%s connected! eng_connect_fd:%d socket_name=%s", __FUNCTION__, eng_connect_fd, socket_name);
    if (0 != pthread_create(&t_eng_soc_rw, NULL, (void *)eng_socket_rw_thread, &eng_connect_fd)) {
        ENG_LOG("eng_socket_rw_thread thread create error\n");
      goto out;
    }

  }

out:
  close(eng_command_server);

  return NULL;
}

int eng_socket_update_to_engmode(char log_type, char location) {
  int c;
  //int connect_fd;
  char update_data[ENG_DATA_LENGTH] = {0};

  ENG_LOG("%s log_type=%d location=%d eng_connect_f=%d", __FUNCTION__, log_type, location, eng_connect_fd);
  update_data[0] = -1;
  update_data[1] = log_type;
  update_data[2] = location;

  c = write(eng_connect_fd, update_data, ENG_DATA_LENGTH);
  ENG_LOG("%s c=%d strerr:%s fd:%d", __FUNCTION__, c, strerror(errno), eng_connect_fd);

  return 0;
}

void* eng_socket_connect_for_test(void *x) {
  int c;
  int connect_fd;
  int i = 0;
  char rev_buf[3] = {0};
  char snd_buf[3] = {0x31, 0x1, 0x1};

  if (connect_socket_local_server(&connect_fd, "engpc_soc.l")) {
    ENG_LOG("eng_soc connect socket error");
    return NULL;
  }

  c = write(connect_fd, snd_buf, 3);

  ENG_LOG("%s snd_buf[]:%d %d %d", __FUNCTION__,snd_buf[0],snd_buf[1],snd_buf[2]);

  ENG_LOG("eng_socket_connect_for_test c=%d", c);

  for (;i<2;i++){
    memset(rev_buf,0,sizeof(rev_buf));
    read(connect_fd,rev_buf,ENG_DATA_LENGTH);
    ENG_LOG("%s rev_buf[]:%d %d %d", __FUNCTION__,rev_buf[0],rev_buf[1],rev_buf[2]);
  }
  return NULL;
}

int eng_socket_connect_for_test_ext(char *buf) {
  eng_thread_t tt;
  if (0 != eng_thread_create(&tt, eng_socket_connect_for_test, NULL)) {
    ENG_LOG("eng_socket_rw_thread thread create error\n");
  }

  return 0;
}
