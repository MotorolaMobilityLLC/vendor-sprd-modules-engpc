#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/rtc.h>
#include "sprd_fts_type.h"
#include "sprd_fts_log.h"

#include <dirent.h>

#define RTC_DEV_FILE		"/dev/rtc0"

#define AT_TSETTIME "AT+TSETTIME="

#define LEAPS_THRU_END_OF(y) ((y)/4 - (y)/100 + (y)/400)

/* Whether it is a leap year */
#define IS_LEAP_YEAR(year)    ((!((year) % 4) && ((year) % 100)) || !((year) % 400))

static const unsigned char rtc_days_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/*
 * The number of days in the month.
 */
static int rtc_month_days(unsigned int month, unsigned int year)
{
	return rtc_days_in_month[month] + (IS_LEAP_YEAR(year) && month == 1);
}

static void sec_to_rtc_time(long time, struct rtc_time *tm)
{
	unsigned int month, year, secs;
	int days;

	days = time / 86400;
	secs = time % 86400;

	/* day of the week, 1970-01-01 was a Thursday */
	tm->tm_wday = (days + 4) % 7;

	year = 1970 + days / 365;
	days -= (year - 1970) * 365
		+ LEAPS_THRU_END_OF(year - 1)
		- LEAPS_THRU_END_OF(1970 - 1);
	if (days < 0) {
		year -= 1;
		days += 365 + IS_LEAP_YEAR(year);
	}
	tm->tm_year = year - 1900;
	tm->tm_yday = days + 1;

	for (month = 0; month < 11; month++) {
		int newdays;

		newdays = days - rtc_month_days(month, year);
		if (newdays < 0)
			break;
		days = newdays;
	}
	tm->tm_mon = month;
	tm->tm_mday = days + 1;

	tm->tm_hour = secs / 3600;
	secs -= tm->tm_hour * 3600;
	tm->tm_min = secs / 60;
	tm->tm_sec = secs - tm->tm_min * 60;

	tm->tm_isdst = 0;
}

static char param[64] = {0};
//static char timestamp_param[64] = {0};
//static char randomnum_param[64] = {0};

#define MAX_LINE 1024

int setKernelTime(long time)
{
	int ret;
	int rtc_fd;
	struct rtc_time tm;

	sec_to_rtc_time(time, &tm);
	ENG_LOG("%s: tm: %d-%d-%d %d:%d:%d", __FUNCTION__,	tm.tm_year + 1900,
		tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	rtc_fd = open(RTC_DEV_FILE, O_RDWR);
	if (rtc_fd < 0) {
		ENG_LOG("%s: open rtc dev file failed.\n", __FUNCTION__);
		return -1;
	}

	ret = ioctl(rtc_fd, RTC_SET_TIME, &tm);
	if (ret < 0) {
		ENG_LOG("%s: set time failed.\n", __FUNCTION__);
	}

	close(rtc_fd);

	return ret;
}

int powInt(int x, int y)
{
    int result = 1;
	int i;
    for ( i = 1; i< y; i++)
    {
        result *= x;
    }
	return result;
}

void hex2Str(unsigned char input[],char output[],int length)
{
    int k;
    int i;
    unsigned char tmp;
    for(k = 0; k < length; k++){
        tmp = input[k];
        for(i = 0;i < 8;i++){
            if(tmp & 0x80) {
                output[k] += powInt(2,8-i);
            }
            tmp = tmp << 1;
        }
    }
    ENG_LOG("hex2Str:output:%s\n", output);
}


int settime(char *buff, char *rsp){ 
	char *ptr = NULL;
    char cmd_buf[256] = {0};
    int ret = -1;
    int nlen = 0;
    
     if (NULL == buff)
    {
        ALOGE("%s,null pointer", __FUNCTION__);
        if(rsp != NULL){
            sprintf(rsp, "\r\nERROR\r\n");
            return strlen(rsp);
        }else{
            return 0;
        }
    }

    if(buff[0] == 0x7e)
    {
        ptr = buff + 1 + sizeof(MSG_HEAD_T);
    }
    else
    {
        ptr = strdup(buff);
    }
    ENG_LOG("%s ptr = %s", __FUNCTION__, ptr);
    
     if (strncasecmp(ptr, AT_TSETTIME,strlen(AT_TSETTIME)) == 0){
        char *ptrpara = ptr+strlen(AT_TSETTIME);
        memset(param, 0, sizeof(param));
        nlen = strlen(buff)-strlen(AT_TSETTIME);
        nlen = (nlen >= sizeof(param)-1)?(sizeof(param)-1):nlen;
        strncpy(param, ptr+strlen(AT_TSETTIME), nlen);
        ENG_LOG("%s full param  = %s", __FUNCTION__, param);
        
        char *timestamp_param = strtok (param,",");
        ENG_LOG("%s timestamp_param  = %s", __FUNCTION__, timestamp_param);
		char *randomnum_param = strtok (NULL,",");
        ENG_LOG("%s trandomnum_param  = %s", __FUNCTION__, randomnum_param);
        
        long unix_stamp = atol(timestamp_param);
        ENG_LOG("%s unix_stamp  = %ld", __FUNCTION__, unix_stamp);

        setKernelTime(unix_stamp);

        sprintf(rsp, "\r\nOK\r\n");
    }else{
        sprintf(rsp, "\r\nERROR\r\n");
    }

    if(buff[0] != 0x7e){
        free(ptr);
        ptr = NULL;
    }

    return strlen(rsp);
	
	
	
	
	}


void register_this_module_ext(struct eng_callback *reg, int *num)
{
    ENG_LOG("register_this_module :settime start\n");
    int moudles_num = 0;

    
    sprintf((reg+moudles_num)->at_cmd, "%s" ,AT_TSETTIME);
     (reg+moudles_num)->eng_linuxcmd_func = settime;
     ENG_LOG("module cmd:%s\n", (reg+moudles_num)->at_cmd);
     moudles_num++;
    
    ENG_LOG("register_this_module :settime MIDDLE.");
    
    *num = moudles_num;
    ENG_LOG("register_this_module :settime end.");
}
