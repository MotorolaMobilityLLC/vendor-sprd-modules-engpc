/*
 * Copyright (C) 2012 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <bootloader_message/bootloader_message.h>
#include "autotest.h"

#define REBOOT_NEW_API 1
static int reboot_into_recovery(const std::vector<std::string>& options) {
    LOGD("Rebooting into recovery");
    std::string err;
    if (!write_bootloader_message(options, &err)) {
    LOGE("Rebooting into recovery fail!");
        return -1;
    }
    property_set("sys.powerctl", "reboot,recovery");
    return 0;
}
static int phone_shutdown_new_API(void)
{
#ifdef LANGUAGE_CN
    const char Cmd1[] = "--wipe_data\n--locale=zh_CN";
#else
    const char Cmd1[] = "--wipe_data\n--locale=en_US";
#endif
    const char Cmd2[] = "--reason=wipe_data_via_recovery\n";
   LOGD("phone_shutdown_new_API Cmd1=%s,Cmd2=%s",Cmd1,Cmd2);
   return reboot_into_recovery({Cmd1, Cmd2});
}

#ifdef __cplusplus
extern "C"{
#endif

int phone_reset_factorytest()
{
    return phone_shutdown_new_API();
}

#ifdef __cplusplus
}
#endif

