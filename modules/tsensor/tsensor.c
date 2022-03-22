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
#include <dirent.h>

#include "sprd_fts_type.h"
#include "sprd_fts_diag.h"
#include "sprd_fts_log.h"

#define TSEN_LOG(fmt, arg...)	ENG_LOG("[TSEN]"fmt, ##arg)

#define BIT(x) (1<<x)
#define PMIC_GLB_DIR "/sys/bus/platform/drivers/sprd-pmic-glb"
#define TSENSOR_DCXO_TEMP "/sys/class/thermal/thermal_zone10/temp"
#define TSENSOR_CRYSTAL_TEMP "/sys/class/thermal/thermal_zone11/temp"
#define PMIC_TYPE(x)	(#x)
typedef struct
{
	char * name;
	int (* restore_default_reg) (void);
}PMIC_TYPE_T;

const char* CONFIG_FILE_SENSOR_PATH = "/sys/class/thermal";
static char osc_path[64];
static char outsen_path[64];
static char pmic_reg_path[100] = {0};
static char pmic_val_path[100] = {0};
static int result = 0;
static bool inited = 0;
static PMIC_TYPE_T *local_pmic;

typedef struct
{
	unsigned int tsx_temp;    // TSX TEMP*100
	unsigned int osc_temp;    //OSC TEMP*100
	unsigned short reserved[2];
}DIAG_AP_PMIC_AFC_TSX_TEMP_T;

static int sc27xx_restore_default_reg(void);
static int ump962x_restore_default_reg(void);

static PMIC_TYPE_T local_pmic_type_set[] = {
	{PMIC_TYPE(sc27xx), sc27xx_restore_default_reg},
	{PMIC_TYPE(ump9622), ump962x_restore_default_reg},
};


/*typedef struct {
  unsigned short status;
  unsigned short length;
  } TOOLS_DIAG_AP_CNF_T;*/

static int ana_read(unsigned int reg_addr, unsigned int * reg_data)
{
	int fd_reg, fd_val, ret;
	char cmds[30] = {0}, ret_val[30] = {0};

	if (reg_data == NULL) {
		TSEN_LOG("%s: input invalid parameter",__FUNCTION__);
		return -1;
	}

	fd_val = open(pmic_val_path, O_RDWR);
	if (fd_val < 0) {
		TSEN_LOG("%s: open \"%s\" fail",__FUNCTION__, pmic_val_path);
		return -1;
	}

	fd_reg = open(pmic_reg_path, O_RDWR);
	if (fd_reg < 0) {
		close(fd_val);
		TSEN_LOG("%s: open \"%s\" fail",__FUNCTION__, pmic_reg_path);
		return -1;
	}

	sprintf(cmds,"%x",reg_addr);
	ret = write( fd_reg, cmds, sizeof(cmds));
	ret = read( fd_val, ret_val,sizeof(ret_val));
	TSEN_LOG("%s: cmds:%s, ret_val:%s",__FUNCTION__,cmds,ret_val);
	close(fd_reg);
	close(fd_val);
	*reg_data = (unsigned int)strtol(ret_val,NULL,16);

	return 0;
}

static int ana_write(unsigned int reg_addr,unsigned int value)
{
	int fd_reg, fd_val;
	char cmds_addr[30] = {0}, cmds_value[30] = {0};

	fd_val = open(pmic_val_path, O_RDWR);
	if (fd_val < 0) {
		TSEN_LOG("%s: open \"%s\" fail",__FUNCTION__, pmic_val_path);
		return -1;
	}

	fd_reg = open(pmic_reg_path, O_RDWR);
	if (fd_reg < 0) {
		close(fd_val);
		TSEN_LOG("%s: open \"%s\" fail",__FUNCTION__, pmic_reg_path);
		return -1;
	}

	sprintf(cmds_addr,"%8x", reg_addr);
	write( fd_reg, cmds_addr, sizeof(cmds_addr));
	sprintf(cmds_value,"%8x", value);
	write( fd_val, cmds_value, sizeof(cmds_value));
	TSEN_LOG("%s: reg_addr:0x%x val:0x%x",__FUNCTION__,reg_addr,value);
	close(fd_reg);
	close(fd_val);
	usleep(10000);

	return 0;
}

static int tsensor_regmap_update_bits(unsigned int reg, unsigned int mask, unsigned int val)
{
	unsigned int tmp, orig;

	if (!pmic_reg_path[0] || !pmic_val_path[0]) {
		TSEN_LOG("Wrong pmic path");
		return -1;
	}

	if (ana_read(reg, &orig) < 0) {
		TSEN_LOG("%s: reg:%x read fail",__FUNCTION__, reg);
		return -1;
	}

	tmp = orig & ~mask;
	tmp |= val & mask;
	if (ana_write(reg, tmp) < 0) {
		TSEN_LOG("%s: reg:%x write fail",__FUNCTION__, reg);
		return -1;
	}

	return 0;
}

static int sc27xx_restore_default_reg(void)
{
#define	SC27XX_TSEN_CTRL0	0x1b34
#define SC27XX_TSEN_CLK_SRC_SEL	BIT(4)
#define	SC27XX_TSEN_ADCLDO_EN	BIT(15)
#define	SC27XX_TSEN_CTRL1	0x1b38
#define SC27XX_TSEN_SDADC_EN	BIT(11)
#define	SC27XX_TSEN_UGBUF_EN	BIT(14)
#define	SC27XX_TSEN_CTRL3	0x1b40
#define	SC27XX_TSEN_EN		BIT(0)
#define	SC27XX_TSEN_SEL_EN	BIT(3)
#define	SC27XX_TSEN_SEL_CH	BIT(4)
#define	SC27XX_TSEN_CTRL4	0x1b44
#define	SC27XX_TSEN_CTRL5	0x1b48

	if (tsensor_regmap_update_bits(SC27XX_TSEN_CTRL0, SC27XX_TSEN_CLK_SRC_SEL, SC27XX_TSEN_CLK_SRC_SEL))
		return -1;
	if (tsensor_regmap_update_bits(SC27XX_TSEN_CTRL3, SC27XX_TSEN_EN | SC27XX_TSEN_SEL_EN | SC27XX_TSEN_SEL_CH, 0))
		return -1;
	if (tsensor_regmap_update_bits(SC27XX_TSEN_CTRL1, SC27XX_TSEN_SDADC_EN | SC27XX_TSEN_UGBUF_EN, 0))
		return -1;
	if (tsensor_regmap_update_bits(SC27XX_TSEN_CTRL0, SC27XX_TSEN_ADCLDO_EN, 0))
		return -1;
	inited = 0;

	return 0;
}

static int ump962x_restore_default_reg(void)
{
#define UMP96XX_TSEN_CTRL0		0x20f8
#define UMP96XX_TSEN_CLK_SRC_SEL	BIT(4)

#define	UMP96XX_TSEN_CTRL3		0x2104
#define UMP96XX_TSEN_SEL_CH		BIT(3)
#define UMP96XX_TSEN_EN			BIT(4)
#define UMP96XX_TSEN_UGBUF_EN		BIT(8)
#define UMP96XX_TSEN_ADCLDO_EN		BIT(12)

#define UMP96XX_TSEN_CTRL1 		0x20fc
#define UMP96XX_TSEN_SDADC_EN		BIT(4)

#define	UMP96XX_TSEN_CTRL6		0x2110
#define UMP96XX_TSEN_SEL_EN		(BIT(6) | BIT(7))

	if (tsensor_regmap_update_bits(UMP96XX_TSEN_CTRL0, UMP96XX_TSEN_CLK_SRC_SEL, UMP96XX_TSEN_CLK_SRC_SEL))
		return -1;
	if (tsensor_regmap_update_bits(UMP96XX_TSEN_CTRL3,
				UMP96XX_TSEN_SEL_CH | UMP96XX_TSEN_EN | UMP96XX_TSEN_UGBUF_EN | UMP96XX_TSEN_ADCLDO_EN, 0))
		return -1;
	if (tsensor_regmap_update_bits(UMP96XX_TSEN_CTRL1, UMP96XX_TSEN_SDADC_EN, 0))
		return -1;
	if (tsensor_regmap_update_bits(UMP96XX_TSEN_CTRL6, UMP96XX_TSEN_SEL_EN, 0))
		return -1;
	inited = 0;

	return 0;
}

static int tsensor_restore_default_reg(void)
{
	if (!local_pmic)
		return -1;

	if (local_pmic->restore_default_reg)
		return local_pmic->restore_default_reg();

	return -1;
}

static int tsensor_match_thm_zone(const char* type)
{
	int i = 0;
	int fd, len;
	char path[64];
	char buf[16];

	while (1){
		snprintf(path, sizeof(path), "%s/thermal_zone%d/type", CONFIG_FILE_SENSOR_PATH, i);
		fd = open(path, O_RDONLY);
		if (fd < 0){
			return -1;
		}
		len = read(fd, buf, 15);
		close(fd);
		if (len < 0){
			return -1;
		}
		buf[len] = '\0';
		TSEN_LOG("get thermal_zone%d type: %s", i, buf);
		if (!strncmp(buf, type, strlen(type))){
			return i;
		}
		++i;
	}
}

static void tsensor_get_thmzone_patch(void)
{
	int i;

	i= tsensor_match_thm_zone("osctsen-thmzone");
	if(i >= 0)
		snprintf(osc_path, sizeof(osc_path), "%s/thermal_zone%d/temp", CONFIG_FILE_SENSOR_PATH, i);
	else
		result = -1;
	i= tsensor_match_thm_zone("outtsen-thmzone");
	if(i >= 0)
		snprintf(outsen_path, sizeof(outsen_path), "%s/thermal_zone%d/temp", CONFIG_FILE_SENSOR_PATH, i);
	else
		result = -1;

}

static int tsensor_get_pmic_path(void)
{
	DIR *dir;
	struct dirent *pdirent;
	char pmic_syscon[16] = {0};
	int i;

	if ((dir = opendir(PMIC_GLB_DIR)) == NULL) {
		TSEN_LOG("Not find PMIC path");
		return -1;
	}

	while ((pdirent = readdir(dir)) != NULL) {
		/* find pmic operation node and match pmic type */
		if (!strstr(pdirent->d_name, "syscon"))
			continue;

		local_pmic = NULL;
		for (i = 0; i < sizeof(local_pmic_type_set) / sizeof(local_pmic_type_set[0]); i++) {
			if (strstr(pdirent->d_name, local_pmic_type_set[i].name)) {
				TSEN_LOG("current pmic type is :%s", local_pmic_type_set[i].name);
				local_pmic = &local_pmic_type_set[i];
				snprintf(pmic_reg_path, sizeof(pmic_reg_path), "%s/%s/pmic_reg",PMIC_GLB_DIR, pdirent->d_name);
				snprintf(pmic_val_path, sizeof(pmic_val_path), "%s/%s/pmic_value",PMIC_GLB_DIR, pdirent->d_name);
				TSEN_LOG("pmic reg path:%s\n", pmic_reg_path);
				TSEN_LOG("pmic value path:%s\n", pmic_val_path);
				break;
			}
		}

		if (local_pmic)
			break;
	}

	if (!pdirent) {
		TSEN_LOG("Not found pmic operation node!!");
		closedir(dir);
		return -1;
	}

	closedir(dir);
	return 0;
}

static int ap_read_tsx_temperature(unsigned int *temp)
{
	int fd;
	int read_len = 0;
	char buffer[64] = {0};

	fd = open(outsen_path, O_RDONLY);

	if (fd >= 0) {
		read_len = read(fd, buffer, sizeof(buffer));
		if (read_len > 0)
			*temp = atoi(buffer);

		close(fd);
	}

	return fd;
}

static int ap_read_osc_temperature(unsigned int *temp)
{
	int fd ;
	int read_len = 0;
	char buffer[64] = {0};

	fd = open(osc_path, O_RDONLY);

	if (fd >= 0) {
		read_len = read(fd, buffer, sizeof(buffer));
		if (read_len > 0)
			*temp = atoi(buffer);

		close(fd);
	}

	return fd;
}

static int eng_diag_read_tsx_osc_temp(char *buf, int len, char *rsp, int rsplen)
{
	TOOLS_DIAG_AP_CNF_T *rsp_status = NULL;
	DIAG_AP_PMIC_AFC_TSX_TEMP_T *tsenor = NULL;
	MSG_HEAD_T *msg_head_ptr;
	char *rsp_ptr;
	unsigned int length;

	if (NULL == buf) {
		TSEN_LOG("%s,null pointer", __FUNCTION__);
		return 0;
	}
	msg_head_ptr = (MSG_HEAD_T *)(buf + 1);
	length = sizeof(MSG_HEAD_T) + sizeof(TOOLS_DIAG_AP_CNF_T) + sizeof(DIAG_AP_PMIC_AFC_TSX_TEMP_T);
	rsp_ptr = (char *)malloc(length);
	if (NULL == rsp_ptr) {
		TSEN_LOG("%s: Buffer malloc failed\n", __FUNCTION__);
		return 0;
	}

	memset(rsp_ptr, '\0', length);
	rsp_status = (TOOLS_DIAG_AP_CNF_T *)(rsp_ptr + sizeof(MSG_HEAD_T));
	rsp_status->status = 0x00;
	tsenor = (DIAG_AP_PMIC_AFC_TSX_TEMP_T *)(rsp_ptr + sizeof(MSG_HEAD_T) + sizeof(TOOLS_DIAG_AP_CNF_T));

	if(result < 0){
		rsp_status->status = 0x01; //error fail
	}

	if(!inited) {
		if(ap_read_tsx_temperature(&tsenor->tsx_temp) < 0){
			TSEN_LOG("%s: read tsx failed\n", __FUNCTION__);
		}
		if(ap_read_osc_temperature(&tsenor->osc_temp) < 0){
			TSEN_LOG("%s: read osc failed\n", __FUNCTION__);
		}
		usleep(480 * 1000);
		inited = 1;
	}

	if(ap_read_tsx_temperature(&tsenor->tsx_temp) < 0){
		rsp_status->status = 0x01; //error fail
	}
	if(ap_read_osc_temperature(&tsenor->osc_temp) < 0){
		rsp_status->status = 0x01; //error fail
	}

	tsenor->reserved[0]=0x0;
	tsenor->reserved[1]=0x0;
	rsp_status->length = sizeof(DIAG_AP_PMIC_AFC_TSX_TEMP_T);

	msg_head_ptr->len = length;
	memcpy(rsp, buf, 1 + sizeof(MSG_HEAD_T));
	memcpy(rsp + 1 + sizeof(MSG_HEAD_T), rsp_status, sizeof(TOOLS_DIAG_AP_CNF_T));
	memcpy(rsp + 1 + sizeof(MSG_HEAD_T) + sizeof(TOOLS_DIAG_AP_CNF_T), tsenor, sizeof(DIAG_AP_PMIC_AFC_TSX_TEMP_T));
	rsp[length + 2 - 1] = 0x7E;
	free(rsp_ptr);

	TSEN_LOG("%s: tsensor tsx_temp %d, osc temp %d\n", __FUNCTION__, tsenor->tsx_temp, tsenor->osc_temp);
	return length + 2;
}

static int eng_diag_restore_default_reg(char *buf, int len, char *rsp, int rsplen)
{
	TOOLS_DIAG_AP_CNF_T *rsp_status = NULL;
	MSG_HEAD_T *msg_head_ptr;
	char *rsp_ptr;
	unsigned int length;

	if (NULL == buf) {
		TSEN_LOG("%s,null pointer", __FUNCTION__);
		return 0;
	}
	msg_head_ptr = (MSG_HEAD_T *)(buf + 1);
	length = sizeof(MSG_HEAD_T) + sizeof(TOOLS_DIAG_AP_CNF_T);
	rsp_ptr = (char *)malloc(length);
	if (NULL == rsp_ptr) {
		TSEN_LOG("%s: Buffer malloc failed\n", __FUNCTION__);
		return 0;
	}

	memset(rsp_ptr, '\0', length);
	rsp_status = (TOOLS_DIAG_AP_CNF_T *)(rsp_ptr + sizeof(MSG_HEAD_T));

	if (tsensor_restore_default_reg())
		rsp_status->status = 0x01; //error fail
	else
		rsp_status->status = 0x00;

	rsp_status->length = 0x00;

	msg_head_ptr->len = length;
	memcpy(rsp, buf, 1 + sizeof(MSG_HEAD_T));
	memcpy(rsp + 1 + sizeof(MSG_HEAD_T), rsp_status, sizeof(TOOLS_DIAG_AP_CNF_T));
	rsp[length + 2 - 1] = 0x7E;
	free(rsp_ptr);

	TSEN_LOG("%s: tsensor registor restore default val \n", __FUNCTION__);
	return length + 2;
}

void register_this_module_ext(struct eng_callback *reg, int *num)
{
	int moudles_num = 0;

	TSEN_LOG("register_this_module_ext: tsensor\n");
	tsensor_get_thmzone_patch();
	tsensor_get_pmic_path();
	TSEN_LOG("osc %s, out %s \n", osc_path, outsen_path);

	(reg + moudles_num)->type = 0x62;
	(reg + moudles_num)->subtype = 0x0;
	(reg + moudles_num)->diag_ap_cmd = 0x31;
	(reg + moudles_num)->eng_diag_func = eng_diag_read_tsx_osc_temp;
	moudles_num++;

	(reg + moudles_num)->type = 0x62;
	(reg + moudles_num)->subtype = 0x0;
	(reg + moudles_num)->diag_ap_cmd = 0x32;
	(reg + moudles_num)->eng_diag_func = eng_diag_restore_default_reg;
	moudles_num++;

	*num = moudles_num;
	TSEN_LOG("register_this_module_ext: %d %d", *num, moudles_num);
}
