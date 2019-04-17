#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "usb.h"
#include "englog.h"
#include "adapter.h"
#include "bootmode.h"

#define ENG_LOG EngLog::info

int disconnect_vbus_charger(void) {
    int fd;
    int ret = -1;

    const char* path = getChargeStopPath();
    if (path == NULL) {
        ENG_LOG("invalid charge stop path.");
        return 1;
    }

    ENG_LOG("charge stop path: %s", path);

    fd = open(path, O_WRONLY);
    if (fd >= 0) {
        ret = write(fd, "1", 2);
        if (ret < 0) {
            ENG_LOG("%s write %s failed! \n", __func__, path);
            close(fd);
            return 0;
        }
        close(fd);
        sleep(1);
    } else {
        ENG_LOG("%s open %s failed! \n", __func__, path);
        return 0;
    }

    return 1;
}

int connect_vbus_charger(void) {
    int fd;
    int ret = -1;

    const char* path = getChargeStopPath();
    if (path == NULL) {
        ENG_LOG("invalid charge stop path.");
        return 1;
    }

    ENG_LOG("charge stop path: %s", path);

    fd = open(path, O_WRONLY);
    if (fd >= 0) {
        ret = write(fd, "0", 2);
        if (ret < 0) {
            ENG_LOG("%s write %s failed! \n", __func__, path);
            close(fd);
            return 0;
        }
        close(fd);
        sleep(1);
    } else {
        ENG_LOG("%s open %s failed! \n", __func__, path);
        return 0;
    }

    return 1;
}

void eng_usb_maximum_speed(USB_DEVICE_SPEED_ENUM speed) {
    int fd = -1;
    int ret = 0;
    char speed_str[10] = {0};

    const char* path = usb_getMaxSpeedPath();
    if (path == NULL) {
        ENG_LOG("invalid usb max speed path.");
        return ;
    }

    ENG_LOG("usb max speed: %d", speed);
    sprintf(speed_str, "%d", speed);    
    fd = open(path, O_WRONLY);
    if (fd >= 0){
        ret = write(fd, speed_str, strlen(speed_str));
        ENG_LOG("%s: Write usb speed=%s success!\n", __FUNCTION__, speed_str);
        close(fd);
    }else{
        ENG_LOG("fail to open %s", path);
    }
}

int eng_usb_state(void) {
    int fd = -1;
    int ret = 0;
    char usb_state[32] = {0};

    const char* path = usb_getStatePath();
    if (path == NULL) {
        ENG_LOG("invalid usb state path.");
        return 1;
    }

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        ret = read(fd, usb_state, 32);
        if (ret > 0) {
            if (0 == strncmp(usb_state, "CONFIGURED", 10)) {
                ret = 1;
            } else {
                ENG_LOG("%s: usb state: %s\n", __FUNCTION__, usb_state);
                ret = 0;
            }
        }
        close(fd);
    } else {
        ret = 0;
        ENG_LOG("%s: Read sys class androidusb state file failed, read:%d\n",__FUNCTION__, ret);
    }

    return ret;
}

int eng_usb_config(char* buff, int nlen){
    return usb_getConf(buff, nlen);
}

int usb_mode(const char* bootmode){
    ENG_LOG("usb_mode: %s", bootmode);
    if (strcasecmp(bootmode, BOOTMODE_CALI) == 0){
        eng_usb_maximum_speed(USB_SPEED_FULL);
        usb_vser_enable(1);
    }else if (strcasecmp(bootmode, BOOTMODE_AUTOTEST) == 0){
        eng_usb_maximum_speed(USB_SPEED_FULL);
        usb_vser_enable(0);
    }else{
        return 0;
    }

    return 0;
}