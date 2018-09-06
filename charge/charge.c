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
#include "eng_diag.h"
#include "eng_modules.h"
#include "engopt.h"
#include "vlog.h"

#define BATTERY_CAPACITY_PATH "/sys/class/power_supply/battery/capacity"
#define CHARGER_STOP_PATH "/sys/class/power_supply/battery/stop_charge"
#define BATTERY_VOL_PATH "/sys/class/power_supply/battery/real_time_voltage"
#define FGU_VOL_FILE_PATH "/sys/class/power_supply/sprdfgu/fgu_vol"

static int vbus_charger_disconnect = 0;

typedef struct {
	unsigned short status;  //==0:success, != 0:fail
	unsigned short length;
} TOOLS_DIAG_AP_STATUS;

typedef struct {
	int capacity;
	int reserved;
} TOOLS_DIAG_AP_CAPACITY;

typedef struct {
	unsigned short cmd;
	unsigned short length;
} TOOLS_DIAG_AP_CMD_T;

typedef struct {
	unsigned char status;
} TOOLS_DIAG_AP_CHARGE_STATUS;

typedef struct {
	unsigned char cmd;
} TOOLS_DIAG_AP_CHARGE_CMD;

typedef struct {
	unsigned int value;
} TOOLS_DIAG_AP_AUX_VOLTAGE;

typedef struct {
	unsigned int value;
} TOOLS_DIAG_AP_FGU_VOLTAGE;

typedef struct {
	unsigned int cmd;
	unsigned int reserved;
} TOOLS_DIAG_AP_FGU_RESERVED;

static int connect_vbus_charger(void)
{
	int fd;
	int ret = -1;

	if (vbus_charger_disconnect == 1) {
		fd = open(CHARGER_STOP_PATH, O_WRONLY);
		if (fd >= 0) {
			ret = write(fd, "0", 2);
			if (ret < 0) {
				ENG_LOG("%s write %s failed! \n", __func__, CHARGER_STOP_PATH);
				close(fd);
				return 0;
			}
			close(fd);
			sleep(1);
			vbus_charger_disconnect = 0;
		} else {
			ENG_LOG("%s open %s failed! \n", __func__, CHARGER_STOP_PATH);
			return 0;
		}
	}

	return 1;
}

static int disconnect_vbus_charger(void)
{
	int fd;
	int ret = -1;

	if (vbus_charger_disconnect == 0) {
		fd = open(CHARGER_STOP_PATH, O_WRONLY);
		if (fd >= 0) {
			ret = write(fd, "1", 2);
			if (ret < 0) {
				ENG_LOG("%s write %s failed! \n", __func__, CHARGER_STOP_PATH);
				close(fd);
				return 0;
			}
			close(fd);
			sleep(1);
			vbus_charger_disconnect = 1;
		} else {
			ENG_LOG("%s open %s failed! \n", __func__, CHARGER_STOP_PATH);
			return 0;
		}
	}

	return 1;
}

static int get_aux_battery_voltage(void)
{
	int fd = -1;
	int read_len = 0;
	char buffer[64] = {0};
	int value = 0;

	fd = open(BATTERY_VOL_PATH, O_RDONLY);
	ENG_LOG("get_aux_battery_voltage fd=%d\n", fd);

	if (fd >= 0) {
		read_len = read(fd, buffer, sizeof(buffer));
		if (read_len > 0) {
			value = atoi(buffer);
		}
		close(fd);
	}

	return value;
}

static int get_fgu_battery_voltage(void)
{
	int fd = -1;
	int read_len = 0;
	char buffer[64] = {0};
	int value = 0;

	fd = open(FGU_VOL_FILE_PATH, O_RDONLY);
	ENG_LOG("get_fgu_battery_voltage fd=%d\n", fd);

	if (fd >= 0) {
		read_len = read(fd, buffer, sizeof(buffer));
		if (read_len > 0) {
			value = atoi(buffer);
		}
		close(fd);
	}

	return value;
}

static int ap_get_aux_voltage(void)
{
	int voltage = 0;
	int i;

	for (i = 0; i < 16; i++) {
		voltage += get_aux_battery_voltage();
	}
	voltage >>= 4;

	return voltage;
}

static int ap_get_fgu_voltage(void)
{
	int voltage = 0;
	int i;

	for (i = 0; i < 16; i++) {
		voltage += get_fgu_battery_voltage();
	}
	voltage >>= 4;

	return voltage;
}

static int eng_diag_control_charge(char *buf, int len, char *rsp, int rsplen)
{
	int ret = 0;
	char *rsp_ptr;
	TOOLS_DIAG_AP_CHARGE_STATUS *aprsp;
	int length;

	if (NULL == buf) {
		ENG_LOG("%s,null pointer", __FUNCTION__);
		return 0;
	}
	MSG_HEAD_T *msg_head_ptr = (MSG_HEAD_T *)(buf + 1);
	TOOLS_DIAG_AP_CHARGE_CMD *charge =
		(TOOLS_DIAG_AP_CHARGE_CMD *)(buf + 1 + sizeof(MSG_HEAD_T));
	length = sizeof(TOOLS_DIAG_AP_CHARGE_CMD) + sizeof(MSG_HEAD_T);
	rsp_ptr = (char *)malloc(length);
	if (NULL == rsp_ptr) {
		ENG_LOG("%s: Buffer malloc failed\n", __FUNCTION__);
		return 0;
	}
	aprsp = (TOOLS_DIAG_AP_CHARGE_STATUS *)(rsp_ptr + sizeof(MSG_HEAD_T) - 1);
	memcpy(rsp_ptr, msg_head_ptr, sizeof(MSG_HEAD_T));
	aprsp->status = 0x01;
	ENG_LOG("control charge_flag->cmd=%d\n", charge->cmd);
	if (0x01 == charge->cmd) {
		ret = connect_vbus_charger();
		if (ret > 0) {
			aprsp->status = 0x00;
		} else {
			aprsp->status = 0x01;
		}
	} else if (0x02 == charge->cmd) {
		ret = disconnect_vbus_charger();
		if (ret > 0) {
			aprsp->status = 0x00;
		} else {
			aprsp->status = 0x01;
		}
	} else if (0x03 == charge->cmd) {
		aprsp->status = 0x00;
	}
	msg_head_ptr->len = 8;
	memcpy(rsp, buf, sizeof(MSG_HEAD_T));
	rsp[length - 1] = aprsp->status;
	rsp[length] = 0x7E;
	free(rsp_ptr);

	return length + 1 ;
}

static int eng_diag_read_aux_voltage(char *buf, int len, char *rsp, int rsplen)
{
	int length;
	char *rsp_ptr;
	int ret = 0;

	if (NULL == buf) {
		ENG_LOG("%s,null pointer", __FUNCTION__);
		return 0;
	}
	MSG_HEAD_T *msg_head_ptr = (MSG_HEAD_T *)(buf + 1);
	length = sizeof(TOOLS_DIAG_AP_AUX_VOLTAGE) + sizeof(MSG_HEAD_T);
	rsp_ptr = (char *)malloc(length);
	if (NULL == rsp_ptr) {
		ENG_LOG("%s: Buffer malloc failed\n", __FUNCTION__);
		return 0;
	}
	memcpy(rsp_ptr, msg_head_ptr, sizeof(MSG_HEAD_T));
	TOOLS_DIAG_AP_AUX_VOLTAGE *charge =
			(TOOLS_DIAG_AP_AUX_VOLTAGE *)(buf + 1 + sizeof(MSG_HEAD_T));
	charge->value = ap_get_aux_voltage();
	ENG_LOG("aux charge->value=%d\n", charge->value);
	msg_head_ptr->len = sizeof(MSG_HEAD_T) + sizeof(TOOLS_DIAG_AP_AUX_VOLTAGE);
	memcpy(rsp, buf, 1 + sizeof(MSG_HEAD_T));
	memcpy(rsp + 1 + sizeof(MSG_HEAD_T), charge, sizeof(TOOLS_DIAG_AP_AUX_VOLTAGE));
	rsp[msg_head_ptr->len + 2 - 1] = 0x7E;
	free(rsp_ptr);

	return msg_head_ptr->len + 2 ;
}

static int eng_diag_read_fgu_voltage(char *buf, int len, char *rsp, int rsplen)
{
	int length;
	char *rsp_ptr;
	int ret = 0;

	if (NULL == buf) {
		ENG_LOG("%s,null pointer", __FUNCTION__);
		return 0;
	}
	MSG_HEAD_T *msg_head_ptr = (MSG_HEAD_T *)(buf + 1);
	length = sizeof(TOOLS_DIAG_AP_FGU_VOLTAGE) + sizeof(TOOLS_DIAG_AP_FGU_RESERVED) + sizeof(MSG_HEAD_T);
	rsp_ptr = (char *)malloc(length);
	if (NULL == rsp_ptr) {
		ENG_LOG("%s: Buffer malloc failed\n", __FUNCTION__);
		return 0;
	}

	memcpy(rsp_ptr, msg_head_ptr, sizeof(MSG_HEAD_T));
	TOOLS_DIAG_AP_FGU_RESERVED *status =
			(TOOLS_DIAG_AP_FGU_RESERVED *)(buf + 1 + sizeof(MSG_HEAD_T));
	TOOLS_DIAG_AP_FGU_VOLTAGE *charge =
			(TOOLS_DIAG_AP_FGU_VOLTAGE *)(buf + 1 + sizeof(TOOLS_DIAG_AP_FGU_RESERVED) + sizeof(MSG_HEAD_T));
	charge->value = ap_get_fgu_voltage();
	ENG_LOG("fgu charge->value=%d\n", charge->value);
	msg_head_ptr->len = len;
	memcpy(rsp, buf, 1 + len);
	memcpy(rsp + 1 + sizeof(MSG_HEAD_T), status, sizeof(TOOLS_DIAG_AP_FGU_RESERVED));
	memcpy(rsp + 1 + sizeof(MSG_HEAD_T) + sizeof(TOOLS_DIAG_AP_FGU_VOLTAGE), charge, sizeof(TOOLS_DIAG_AP_FGU_VOLTAGE));
	rsp[msg_head_ptr->len + 2 - 1] = 0x7E;
	free(rsp_ptr);

	return msg_head_ptr->len + 2 ;
}

static int eng_diag_get_battery_capacity(char *buf, int len, char *rsp, int rsplen)
{
	TOOLS_DIAG_AP_STATUS *rsp_status = NULL;
	TOOLS_DIAG_AP_CAPACITY *bat_info = NULL;
	MSG_HEAD_T *msg_head_ptr;
	unsigned short read_len;
	char buffer[32] = {0};
	char *rsp_ptr;
	int cap;
	int fd = -1;

	if (NULL == buf) {
		ENG_LOG("%s,null pointer", __FUNCTION__);
		return 0;
	}
	msg_head_ptr = (MSG_HEAD_T *)(rsp + 1);
	rsplen = sizeof(MSG_HEAD_T) + sizeof(TOOLS_DIAG_AP_STATUS) + sizeof(TOOLS_DIAG_AP_CAPACITY);
	rsp_ptr = (char *)malloc(rsplen);
	if (NULL == rsp_ptr) {
		ENG_LOG("%s: Buffer malloc failed\n", __FUNCTION__);
		return 0;
	}

	memset(rsp_ptr, '\0', rsplen);
	rsp_status = (TOOLS_DIAG_AP_STATUS *)(rsp_ptr + sizeof(MSG_HEAD_T));
	bat_info = (TOOLS_DIAG_AP_CAPACITY *)(rsp_ptr + sizeof(MSG_HEAD_T) + sizeof(TOOLS_DIAG_AP_STATUS));

	fd = open(BATTERY_CAPACITY_PATH, O_RDONLY);
	if (fd >= 0) {
		read_len = read(fd, buffer, sizeof(buffer));
		if (read_len > 0) {
			bat_info->capacity = atoi(buffer);
			rsp_status->status = 0x00;
			rsp_status->length = sizeof(TOOLS_DIAG_AP_CAPACITY);
		}
		close(fd);
		ENG_LOG("%s %s capacity = %d, read_len = %d\n", __FUNCTION__,
				BATTERY_CAPACITY_PATH, bat_info->capacity, read_len);
	} else {
		rsp_status->status = 0x01;
		rsp_status->length = 0x00;
		ENG_LOG("%s open %s error!!!\n", __FUNCTION__, BATTERY_CAPACITY_PATH);
	}

	buf[5] = len - 2 + sizeof(TOOLS_DIAG_AP_CAPACITY);
	msg_head_ptr->len = buf[5];
	memcpy(rsp, buf, 1 + sizeof(MSG_HEAD_T));
	memcpy(rsp + 1 + sizeof(MSG_HEAD_T), rsp_status, sizeof(TOOLS_DIAG_AP_STATUS));
	memcpy(rsp + 1 + sizeof(MSG_HEAD_T) + sizeof(TOOLS_DIAG_AP_STATUS), &bat_info->capacity, sizeof(TOOLS_DIAG_AP_CAPACITY));
	rsp[msg_head_ptr->len + 2 - 1] = 0x7E;
	free(rsp_ptr);

	return msg_head_ptr->len + 2;
}

void register_this_module_ext(struct eng_callback *reg, int *num)
{
	int moudles_num = 0;

	ENG_LOG("register_this_module_ext: charger\n");
	reg->type = 0x62;
	reg->subtype = 0x0;
	reg->diag_ap_cmd = 0x30;
	reg->eng_diag_func = eng_diag_get_battery_capacity;
	moudles_num++;

	(reg + moudles_num)->type = 0x38;
	(reg + moudles_num)->subtype = 0x12;
	(reg + moudles_num)->eng_diag_func = eng_diag_control_charge;
	moudles_num++;

	(reg + moudles_num)->type = 0x1e;
	(reg + moudles_num)->subtype = 0x00;
	(reg + moudles_num)->eng_diag_func = eng_diag_read_aux_voltage;
	moudles_num++;

	(reg + moudles_num)->type = 0x62;
	(reg + moudles_num)->subtype = 0x00;
	(reg + moudles_num)->eng_diag_func = eng_diag_read_fgu_voltage;
	moudles_num++;

	*num = moudles_num;
	ENG_LOG("register_this_module_ext: %d %d", *num, moudles_num);
}
