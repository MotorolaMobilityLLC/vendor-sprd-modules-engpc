#include "engopt.h"
#include "eng_debug.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include "private/android_filesystem_config.h"

int eng_file_lock(void) {
  int fd;
  if ((fd = open("/data/vendor/local/englog/file_lock.test", O_CREAT | O_RDONLY,
                 0444)) != -1) {
    ENG_LOG("open data/vendor/local/englog/file_lock.test fd = %d\n", fd);
    if (flock(fd, LOCK_EX | LOCK_NB) != -1) {
      ENG_LOG("lock /data/vendor/local/englog/file_lock.test \n");
    }
    return fd;
  } else {
    ENG_LOG("open data/vendor/local/englog/file_lock.test fail %s\n", strerror(errno));
  }
  return -1;
}

int eng_file_unlock(int fd) {
  // flock(fd,LOCK_UN);
  ENG_LOG("unlock /data/vendor/file_lock.test \n");
  if (fd >= 0) close(fd);

  return 0;
}

void* eng_printlog_thread(void* x) {
  int ret = -1;
  int fd = -1;

  ENG_LOG("eng_printlog_thread thread start\n");

  if (0 != access("/data/vendor/local/englog", F_OK)) {
    ret = mkdir("/data/vendor/local/englog", S_IRWXU | S_IRWXG | S_IRWXO);
    if (-1 == ret && (errno != EEXIST)) {
      ENG_LOG("mkdir /data/vendor/local/englog failed.");
      return 0;
    }
  }
  ret = chmod("/data/vendor/local/englog", S_IRWXU | S_IRWXG | S_IRWXO);
  if (-1 == ret) {
    ENG_LOG("chmod /data/vendor/local/englog failed.");
    return 0;
  }

  if (0 == access("/data/vendor/local/englog/last_eng.log", F_OK)) {
    ret = remove("/data/vendor/local/englog/last_eng.log");
    if (-1 == ret) {
      ENG_LOG("remove failed.");
      return 0;
    }
  }

  if (0 == access("/data/vendor/local/englog/eng.log", F_OK)) {
    ret =
        rename("/data/vendor/local/englog/eng.log", "/data/vendor/local/englog/last_eng.log");
    if (-1 == ret) {
      ENG_LOG("rename failed.");
      return 0;
    }
  }

  fd = open("/data/vendor/local/englog/eng.log", O_RDWR | O_CREAT,
            S_IRWXU | S_IRWXG | S_IRWXO);
  if (fd == -1 && (errno != EEXIST)) {
    ENG_LOG("creat /data/vendor/local/englog/eng.log failed.");
    return 0;
  }
  if (fd >= 0) close(fd);

  ret = chmod("/data/vendor/local/englog/eng.log", 0777);
  if (-1 == ret) {
    ENG_LOG("chmod /data/vendor/local/englog/eng.log failed.");
    return 0;
  }

  ret = system("logcat -v threadtime -f /data/vendor/local/englog/eng.log -s ENGPC &");
  if (!WIFEXITED(ret) || WEXITSTATUS(ret) || -1 == ret) {
    ENG_LOG(" system failed.");
    return 0;
  }

  system("sync");

  return 0;
}
