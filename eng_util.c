#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "eng_util.h"
#include "engopt.h"

#ifdef MAX_OPEN_TIMES
#undef MAX_OPEN_TIMES
#endif
#define MAX_OPEN_TIMES 100

int parse_text_delims(char* input_text,char *output_text1,char *output_text2,char *delims){
    char *p;
    char *buffer;
    int count = 0;
    int cmd_len = 0;
    buffer = strdup(input_text);
    count = strlen(buffer) + 1;
    ENG_LOG("%s: buffer=%s",__FUNCTION__, buffer);
    p = strtok(buffer,delims);
    if(p != NULL && output_text1 != NULL){
            cmd_len = strlen(p) + 1;
            snprintf(output_text1, cmd_len, "%s", p);
    }
    if(buffer != NULL && output_text2 != NULL){
        ENG_LOG("%s: buffer count=%d,cmd_len=%d",__FUNCTION__, count,cmd_len);
        strncpy(output_text2, buffer + cmd_len, count - cmd_len);
    }
    ENG_LOG("%s: output_text1=%s,output_text2=%s",__FUNCTION__, output_text1,output_text2);
    return 0;
}

int eng_open_dev(char* dev, int mode) {
  int fd;
  struct termios ser_settings;

  fd = open(dev, mode);
  if (fd < 0) return -1;

  if (isatty(fd)) {
    tcgetattr(fd, &ser_settings);
    cfmakeraw(&ser_settings);
    tcsetattr(fd, TCSANOW, &ser_settings);
  }

  return fd;
}

/**
 * command interface: send data to lte diag
 * @nmea:        data buffer
 * @length:     data buffer length
 *
 * If operation succeed, return data buffer length, otherwise, return -1.
 */
int write_to_host_diag(char* nmea, int length) {
  int r_cnt = 0, w_cnt = 0, offset = 0;
  int retry_num = 0;
  int ser_fd;
  char* diag_data = NULL;

  ENG_LOG("%s: length=%d \n", __FUNCTION__, length);

  ser_fd = get_ser_diag_fd();

  if (length >= 2) {
    diag_data = (char *)malloc(length * 2);
    if (NULL == diag_data) {
      ENG_LOG("%s: Buffer malloc failed\n", __FUNCTION__);
      goto err;
    }

    r_cnt = translate_packet(diag_data, nmea + 1, length - 2);

    offset = 0;
    w_cnt = 0;
    do {
      w_cnt = write(ser_fd, diag_data + offset, r_cnt);
      ENG_LOG("%s: w_cnt = %d\n r_cnt = %d offset = %d", __FUNCTION__, w_cnt, r_cnt, offset);
      if (w_cnt < 0) {
        if (errno == EBUSY) {
          usleep(59000);
        } else {
          retry_num = 0;
          while (-1 == restart_gser(&ser_fd, get_ser_diag_path())) {
            ENG_LOG("eng_gps_log open ser port failed\n");
            sleep(1);
            retry_num++;
            if (retry_num > MAX_OPEN_TIMES) {
              ENG_LOG("eng_gps_log: thread stop for open ser error!\n");
              goto err;
            }
          }
          update_ser_diag_fd(ser_fd);
        }
      } else {
        r_cnt -= w_cnt;
        offset += w_cnt;
      }
    } while (r_cnt > 0);
  }

  free(diag_data);
  diag_data = NULL;
  return length;

err:
  if (NULL != diag_data){
    free(diag_data);
    diag_data = NULL;
  }
  return -1;
}