#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>

#include "englog.h"
#include "CDevMgr.h"
#include "CChnlThreadMgr.h"
#include "CChnlMgr.h"
#include "./module/CModuleMgr.h"
#include "bootmode.h"
#include "adapter.h"

#define CHNL_DEV_DIR_PATH "/vendor/etc/engpc/dev"
#define CHNL_CHNL_DIR_PATH "/vendor/etc/engpc/chnl"
#define DYNAMIC_SO_DIR_PATH "/vendor/lib/npidevice"

#define DEV_HOST_NAME "pc"
#define DEV_HOST_DIAG_NAME "COM_CP_DIAG"

#define SAFE_DELETE(s) {if (s != NULL) delete s;}

void* eng_printlog_thread(void* x);
int wait_for_modem_alive(int timeout);
void wait_for_data_ready();
void log_start();

CModuleMgr* lpModMgr = NULL;
CDevMgr* lpDevMgr = NULL;

int main(){

    EngLog::info("main...");

    const char* bootmode = initBootMode();
    EngLog::info("bootmode = %s", bootmode);

    if (strcasecmp(bootmode, BOOTMODE_CALI) == 0 || strcasecmp(bootmode, BOOTMODE_AUTOTEST) == 0){
        log_start();
    }

    EngLog::info("load dynamic so");
    lpModMgr = CModuleMgr::getInstance(DYNAMIC_SO_DIR_PATH);
    if (NULL == lpModMgr || 0 != lpModMgr->load()){
        EngLog::error("load dynamic so fail");
    }

    EngLog::info("load dev manager...");
    lpDevMgr = CDevMgr::getInstance();
    if (NULL == lpDevMgr || 0 != lpDevMgr->load((char*)CHNL_DEV_DIR_PATH)){
        EngLog::error("load devmgr fail.");
        SAFE_DELETE(lpModMgr);
        return -1;
    }
    lpDevMgr->print();

    EngLog::info("load thread manager...");
    CChnlThreadMgr* lpThreadMgr = CChnlThreadMgr::getInstance();
    if (NULL == lpThreadMgr){
        EngLog::error("init threadmgr fail.");
        SAFE_DELETE(lpModMgr);
        SAFE_DELETE(lpDevMgr);
        return -1;
    }

    EngLog::info("load chnl manager...");
    CChnlMgr* lpChnlMgr = CChnlMgr::getInstance(lpThreadMgr, lpDevMgr, lpModMgr);
    if (NULL == lpChnlMgr || 0 != lpChnlMgr->load((char*)CHNL_CHNL_DIR_PATH)){
        EngLog::error("init chnlmgr fail.");
        SAFE_DELETE(lpModMgr);
        SAFE_DELETE(lpDevMgr);
        SAFE_DELETE(lpThreadMgr);
        return -1;
    }
    lpChnlMgr->print();

    EngLog::info("enable %s", bootmode);
    lpChnlMgr->enable(bootmode, true);
    EngLog::info("active mode");
    lpDevMgr->activeMode((char* )bootmode);
    EngLog::info("set host dev diag port");
    CDev* lpDev = lpDevMgr->find(DEV_HOST_NAME);
    if (lpDev != NULL){
        lpModMgr->regDiagHost(lpDev->find(DEV_HOST_DIAG_NAME));
    }else{
        EngLog::info("can not find host dev diag port.");
    }

#ifndef ENGPC_AP_CALI

    EngLog::info("wait for modem alive");
    if (0 != wait_for_modem_alive(60)){
        EngLog::error("wait modem alive fail!!!!!!!!! please check modem status!");
    }

#endif

    if (strcasecmp(bootmode, BOOTMODE_NORMAL) == 0
        || strcasecmp(bootmode, BOOTMODE_NORMAL_LITE) == 0
        || strcasecmp(bootmode, BOOTMODE_AUTOTEST) == 0){
        //wait_for_data_ready();
    }

    usb_mode(bootmode);

    EngLog::info("run...");
    lpChnlMgr->run(bootmode);

    while(1){
        sleep((unsigned int)(-1));
    }

    return 1;
}

void wait_for_data_ready(){
    int count = 60;
    while(!is_data_dir_ready() && count-- > 0){
        sleep(1);
    }
    if (count <= 0){
        EngLog::error("warning!!!!!!!!!!wait for encrypt fail.");
    }
}

int wait_for_modem_alive(int timeout)
{
    int fd = -1;
    int try_cnt = timeout;

    EngLog::info("timeout = %d", timeout);

    CDev* lpDev = lpDevMgr->find("cp");
    if (lpDev != NULL){
        CPort* lpPort = lpDev->find("COM_DIAG");
        if (lpPort != NULL){
            char* path = (char* )(lpPort->getpath());
            EngLog::info("path = %s", path);
            do {
                fd = open(path, O_RDWR | O_NONBLOCK);
                if (fd >= 0) {
                    break;
                }
                EngLog::info("%s: times:%d, failed to open tty dev:  %s, fd = %d",__FUNCTION__, try_cnt, path, fd);
                usleep(1000 * 1000);
                try_cnt--;
            } while (try_cnt > 0);
        
            if (fd >= 0){
                EngLog::info("%s: times:%d, succ open tty dev:  %s, fd = %d",__FUNCTION__, try_cnt, path, fd);
                close(fd);
                return 0;
            }else{
                return -1;
            }
        }else{
            EngLog::error("can't find port: COM_DIAG");
        }
    }else{
        EngLog::error("can't find dev: cp");
    }

    return 0;
}

void* eng_printlog_thread(void* x) {
  int ret = -1;
  int fd = -1;

  EngLog::error("eng_printlog_thread thread start\n");

  if (0 != access("/data/local/englog", F_OK)) {
    ret = mkdir("/data/local/englog", S_IRWXU | S_IRWXG | S_IRWXO);
    if (-1 == ret && (errno != EEXIST)) {
      EngLog::error("mkdir /data/local/englog failed.");
      return 0;
    }
  }
  ret = chmod("/data/local/englog", S_IRWXU | S_IRWXG | S_IRWXO);
  if (-1 == ret) {
    EngLog::error("chmod /data/local/englog failed.");
    return 0;
  }

  if (0 == access("/data/local/englog/last_eng.log", F_OK)) {
    ret = remove("/data/local/englog/last_eng.log");
    if (-1 == ret) {
      EngLog::error("remove failed.");
      return 0;
    }
  }

  if (0 == access("/data/local/englog/eng.log", F_OK)) {
    ret =
        rename("/data/local/englog/eng.log", "/data/local/englog/last_eng.log");
    if (-1 == ret) {
      EngLog::error("rename failed.");
      return 0;
    }
  }

  fd = open("/data/local/englog/eng.log", O_RDWR | O_CREAT,
            S_IRWXU | S_IRWXG | S_IRWXO);
  if (fd == -1 && (errno != EEXIST)) {
    EngLog::error("creat /data/local/englog/eng.log failed.");
    return 0;
  }
  if (fd >= 0) close(fd);

  ret = chmod("/data/local/englog/eng.log", 0777);
  if (-1 == ret) {
    EngLog::error("chmod /data/local/englog/eng.log failed.");
    return 0;
  }

  ret = system("logcat -v threadtime -f /data/local/englog/eng.log &");
  if (!WIFEXITED(ret) || WEXITSTATUS(ret) || -1 == ret) {
    EngLog::error(" system failed.");
    return 0;
  }

  system("sync");

  return 0;
}

void log_start(){
    pthread_t t;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&t, &attr, eng_printlog_thread, NULL);
}
