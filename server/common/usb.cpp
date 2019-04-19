#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include <cutils/uevent.h>

#include "usb.h"
#include "englog.h"
#include "adapter.h"
#include "bootmode.h"
#include "CDevMgr.h"

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

static uevent_notify s_ptrNotify = NULL;

void usb_monitor(uevent_notify ptrNotify){
    pthread_t t;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    ENG_LOG("usb_monitor: ptf = %x", ptrNotify);
    s_ptrNotify = ptrNotify;

    if (0 != pthread_create(&t, &attr, eng_uevt_thread, NULL)){
        ENG_LOG("usb monitor fail.");
    }
}

void parse_event(const char *msg, struct uevent *uevent) {
    uevent->action = "";
    uevent->path = "";
    uevent->subsystem = "";
    uevent->usb_connect = "";

    while (*msg) {
        if (!strncmp(msg, "ACTION=", 7)) {
            msg += 7;
            uevent->action = msg;
        } else if (!strncmp(msg, "DEVPATH=", 8)) {
            msg += 8;
            uevent->path = msg;
        } else if (!strncmp(msg, "SUBSYSTEM=", 10)) {
            msg += 10;
            uevent->subsystem = msg;
        } else if (!strncmp(msg, "USB_STATE=", 10)) {
            msg += 10;
            uevent->usb_connect = msg;

            ENG_LOG("%s: event { '%s', '%s', '%s', '%s' }\n", __FUNCTION__,
                    uevent->action, uevent->path, uevent->subsystem, uevent->usb_connect);
        }
        
        /* advance to after the next \0 */
        while (*msg++)
            ;
    }
}

extern CDevMgr* g_lpDevMgr;

void handle_device_event(struct uevent *uevent) {
    if (0 == strncmp(uevent->usb_connect, "CONFIGURED", 10)) {
        // start cp log
        ENG_LOG("%s: enable arm log\n", __FUNCTION__);
        if (s_ptrNotify != NULL){
            s_ptrNotify(USB_CONNECT, (void*)g_lpDevMgr);
        }
    } else if (0 == strncmp(uevent->usb_connect, "DISCONNECTED", 12)) {
        // stop cp log
        ENG_LOG("%s: disable arm log\n", __FUNCTION__);
        if (s_ptrNotify != NULL){
            s_ptrNotify(USB_DISCONNECT, (void*)g_lpDevMgr);
        }
    }
}

void handle_device_fd(int sock) {
    char msg[UEVENT_MSG_LEN + 2];
    int n;
    while ((n = uevent_kernel_multicast_recv(sock, msg, UEVENT_MSG_LEN)) > 0) {
        if (n >= UEVENT_MSG_LEN) /* overflow -- discard */
            continue;
        
        msg[n] = '\0';
        msg[n + 1] = '\0';
        
        struct uevent uevent;
        parse_event(msg, &uevent);
        handle_device_event(&uevent);
    }
}

void *eng_uevt_thread(void *x) {
    struct pollfd ufd;
    int sock = -1;
    int nr;

    sock = uevent_open_socket(256 * 1024, true);
    if (-1 == sock) {
        ENG_LOG("%s: socket init failed !\n", __FUNCTION__);
        return 0;
    }

    ufd.events = POLLIN;
    ufd.fd = sock;
    while (1) {
        ufd.revents = 0;
        nr = poll(&ufd, 1, -1);
        if (nr <= 0)
            continue;
        if (ufd.revents == POLLIN) 
            handle_device_fd(sock);
    }

    return 0;
}
