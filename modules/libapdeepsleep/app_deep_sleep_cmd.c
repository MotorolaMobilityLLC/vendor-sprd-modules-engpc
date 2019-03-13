#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "eng_modules.h"
#include "engopt.h"

#define SYS_CLASS_ANDUSB_STATE "/sys/class/android_usb/android0/state"

int ap_ds_usb_state(void) {
  int fd = -1;
  int ret = 0;
  char usb_state[32] = {0};

  fd = open(SYS_CLASS_ANDUSB_STATE, O_RDONLY);
  if (fd >= 0) {
    ret = read(fd, usb_state, 32);
    if (ret > 0) {
      if (0 == strncmp(usb_state, "CONFIGURED", 10)) {
        ret = 1;
      } else {
        //ENG_LOG("%s: usb state: %s\n", __FUNCTION__, usb_state);
        ret = 0;
      }
    } else {
      ret = 0;
      //ENG_LOG("%s: Read sys class androidusb state file failed, read:%d\n",
              //__FUNCTION__, ret);
    }

    close(fd);
  } else {
    ret = 0;
    //ENG_LOG("%s: Open sys class androidusb state file failed, err: %s!\n",
            //__FUNCTION__, strerror(errno));
  }

  return ret;
}

void *ap_ds_thread_fastsleep(void *para) {

  int count;
  int sleep = 0;
  char cmd[] = {"echo mem > /sys/power/autosleep"};

  ENG_LOG("[apdeepsleep]##: please plug out usb within 60s...\n");
  for (count = 0; count < 60*5; count ++) {
    if (!ap_ds_usb_state()){
      sleep = 1;
      break;
    }
    usleep(200*1000);
  }
  ENG_LOG("[apdeepsleep]##: sleep count=%d\n", count);
  if (sleep) {
    ENG_LOG("[apdeepsleep]##: going to sleep mode!delay 3s\n");
    usleep(3000*1000);
    system(cmd);
  }

  return NULL;
}

// return : rsp true length
int ap_deep_sleep_handler(char *buf, int len, char *rsp, int rsplen){
  pthread_t thread_id;
  int ret;
  ret = pthread_create(&thread_id, NULL, ap_ds_thread_fastsleep, NULL);
  if (0 != ret) {
    //ENG_LOG("%s: Can't create thread[thread_fastsleep]!\n", __FUNCTION__);
  } else {
    //ENG_LOG("%s: Create thread[thread_fastsleep] sucessfully!\n", __FUNCTION__);
  }
   //deep sleep : rsp is not needed upon ap side
   return 2;
}



void register_this_module_ext(struct eng_callback *reg, int *num)
{
    int moudles_num = 0;
    //ENG_LOG("register_this_module_ext :libapdeepsleep");

    //1st command
    reg->type = 0x11; //main cmd
    reg->subtype = 0x2; //sub cmd
    reg->also_need_to_cp = 1;  //deep sleep cmd is also dealed with upon cp side
    reg->eng_diag_func = ap_deep_sleep_handler; // rsp function ptr
    moudles_num++;

    *num = moudles_num;
    //ENG_LOG("register_this_module_ext: %d - %d",*num, moudles_num);
}
