#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <ctype.h>
#include <utils/Log.h>
#include "../../vlog.h"
#include "../../eng_diag.h"
#include "../pq_minui/graphics.h"
#include "tune_lite_r2p0.h"

struct ops_entry tune_lite_r2p0_entry;
static struct tune_context ctx;

static int MultTimes(int base, int n)
{
	int mt = 1;
	int m;

	for(m = 0; m < n; m++)
		mt *= base;
	return mt; 
}

static void parse_panelsize(char *data,u32 count,DUT_INFO_T  *dut_info_t)
{
	unsigned char i;
	unsigned char j;
	unsigned char next = 0;
	unsigned char last = 0;

	dut_info_t->stResolution.HPixel = 0;
	dut_info_t->stResolution.VPixel = 0;
	for (i = 0; i< count; i++){
		if (data[i] == 'x'){
			next = i + 1;
			for (j=0; j < i-1; j++){
				dut_info_t->stResolution.HPixel += (data[j]- 48)*(MultTimes(10, i-1-j));
			}   
		}   
		if(data[i] == '\n'){
			last = i -1; 
			for(j = 0; j < (last-next+1); j++){
				dut_info_t->stResolution.VPixel += (data[next+j]- 48)*(MultTimes(10, last-next-j));
			}   

		}
	}   

}

static int tune_connect(char *buf, int len, char *rsp, int rsplen)
{
	DUT_INFO_T  *dut_info;
	MSG_HEAD_T *rsp_head;
	u32 rsp_len = 0;
	char info[1024] = {0};
	char backlight[10] = {0};
	u32 sizes = 0;
	int fd;
	int fd1;
	int fd2;
	int fd3;
	char *pchar;
	char *ptemp;
	unsigned long status;
	int ret;

	ret = gr_init();
	if(ret)
		ENG_LOG("PQ display init fail\n");;

	fd = open(PanelSize, O_RDONLY);
	fd1 = open(ChipInfo, O_RDONLY);
	fd2 = open(PQStatus, O_RDONLY);
	fd3 = open(Brightness, O_RDWR);
	if (fd < 0 || fd1 < 0 || fd2 < 0 || fd3 < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__,
			strerror(errno));
		return errno;
	}

	snprintf(backlight, sizeof(backlight), "%d", 255);
	write(fd3, backlight, sizeof(backlight));

	dut_info =  (DUT_INFO_T  *)malloc(sizeof(DUT_INFO_T));
	if(!dut_info) {
		ENG_LOG("PQ dut info alloc fail\n");
		return -1;
	}

	ret = read(fd2, info, 100);
	if (ret < 0) {
		ENG_LOG("PQ display sleep in\n");
		ctx.params.gamma.version.enable = 0;
		ctx.params.abc.version.enable = 0;
		ctx.params.cms.version.enable = 0;
		ctx.params.bld.version.enable = 0;
	} else if (ret > 0) {
		pchar = strstr(info, "0x");
		if (pchar) {
			status = strtol(pchar, NULL, 16);
			if (status & GAMMA_EN)
				ctx.params.gamma.version.enable = 1;
			if (status & SLP_EN)
				ctx.params.abc.version.enable = 1;
			if (status & CMS_EN)
				ctx.params.cms.version.enable = 1;
			if (status & HSV_EN)
				ctx.params.bld.version.enable = 1;
		}
	}
	ENG_LOG("PQ status %x\n", status);
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	memset(dut_info, 0, sizeof(DUT_INFO_T));
	sizes = read(fd1, info, 1024);
	pchar = strstr(info, "androidboot.hardware");
	pchar = strstr(pchar, "=");
	ptemp = strchr(pchar + 1, ' ');
	sizes = ptemp - pchar - 1;
	strncpy(dut_info->szModelName, tune_lite_r2p0_entry.version, strlen(tune_lite_r2p0_entry.version));
	strncpy(dut_info->szChipName, pchar + 1, sizes);
	sizes = read(fd, info, 1024);
	parse_panelsize(info, sizes , dut_info);
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	memcpy(rsp + DIAG_HEADER_LENGTH + 5, dut_info, sizeof(DUT_INFO_T));
	rsp_len = DIAG_HEADER_LENGTH + 6 + sizeof(DUT_INFO_T);
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;
	free(dut_info);
	close(fd);
	close(fd1);
	close(fd2);
	close(fd3);
	ENG_LOG("PQ pq_cmd_connect sucess v5\n");
	return rsp_len;
}

static int tune_rgb_pattern(char *buf, int len, char *rsp, int rsplen)
{
	int fd;
	int fd0;
	int ret = 0;
	char rgb[10] = {0};
	char flip[10] = {0};
	bool exit = false;
	static bool flip_en = true;
	u32 *pdata = NULL;
	u32 *rsp_pdata = NULL;
	MSG_HEAD_T *rsp_head;
	u32 rsp_len = 0;
	u32 extra_len = 0;

	fd = open(DispcBg, O_RDWR);
	fd0 = open(DpuRefresh, O_RDWR);
	if (fd < 0 || fd0 < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	if(flip_en) {
		sprintf(flip, "%d", 1);
		write(fd0, flip, sizeof(flip));
		flip_en = false;
	}

	pdata = (u32 *)(buf + DIAG_HEADER_LENGTH + 5);
	rsp_pdata = (u32 *)(rsp + DIAG_HEADER_LENGTH + 5);;
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	if (*pdata != 0xffffffff) {
		sprintf(rgb, "%x", *pdata);
		ret = write(fd, rgb, sizeof(rgb));
		exit = false;
	} else {
		sprintf(flip, "%d", 1);
		write(fd0, flip, sizeof(flip));
		ret = 0;
		exit = true;
		flip_en = true;
	}
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	if (ret == -1)
		*rsp_pdata = 1;
	else
		*rsp_pdata = 0;
	rsp_len = DIAG_HEADER_LENGTH + 10;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;
	close(fd);
	close(fd0);
	return rsp_len;

}

static int tune_read_regs(char *buf, int len, char *rsp, int rsplen)
{
	int fd0;
	int fd1;
	u32 *pdata;
	u32 offset;
	u32 length;
	u32 sizes = 0;
	u32 rsp_len = 0;
	char cmds[30];
	char *bufs;
	u32 extra_len = 0;
	int ret = 0;
	MSG_HEAD_T *rsp_head;

	fd0 = open(DispcRegsOffset, O_RDWR);
	fd1 = open(DispcWrRegs, O_RDWR);
	if (fd0 < 0 || fd1 < 0) {
			ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	pdata = (u32 *)(buf + DIAG_HEADER_LENGTH + 5);
	offset = pdata[1];
	length = pdata[0];
	ENG_LOG("PQ %s: offset = %x len = %x\n", __func__, offset, length);
	bufs = (char *)(rsp + DIAG_HEADER_LENGTH + 13);
	sizes = sprintf(cmds, "%8x ", offset);
	sprintf(cmds + sizes, "%8x ", length);
	ret = write(fd0 ,cmds, sizeof(cmds));
	ret = read(fd1, bufs, 9 * length);
	HexStrstoInt(bufs, length);
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	rsp_len = DIAG_HEADER_LENGTH + 14 + length*4;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;
	close(fd0);
	close(fd1);
	return rsp_len;
}

static int tune_write_regs(char *buf, int len, char *rsp, int rsplen)
{
	int fd0;
	int fd1;
	u32 *pcmd;
	int offset;
	int length;
	u32 rsp_len = 0;
	char *cmds;
	int sizes = 0;
	int data_len;
	int *pdata;
	int i = 0;
	int err_rsp = 0;
	int ret = 0;
	u32 extra_len = 0;
	MSG_HEAD_T *rsp_head;
	MSG_HEAD_T *send_head;

	fd0 = open(DispcRegsOffset, O_RDWR);
	fd1 = open(DispcWrRegs, O_RDWR);
	if ((fd0 < 0) || (fd1 < 0)) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	cmds = (rsp + DIAG_HEADER_LENGTH + 5);
	pcmd = (u32 *)(buf + DIAG_HEADER_LENGTH + 5);
	offset = pcmd[1];
	length = pcmd[0];
	ENG_LOG("PQ %s  offset = %x, length = %x \n", __func__, offset, length);
	sizes = sprintf(cmds, "%8x ", offset);
	sizes += sprintf(cmds + sizes, "%8x ", length);
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	send_head = (MSG_HEAD_T *)(buf + 1);
	data_len = length;//send_head->len - 12 - DIAG_HEADER_LENGTH;
	ENG_LOG("PQ %s data_len = %x \n", __func__, data_len);
	pdata = (int*)(buf + DIAG_HEADER_LENGTH + 13);
	ENG_LOG("PQ %s pdata = %x \n", __func__, *pdata);
	ret = write(fd0 ,cmds, sizes);
	sizes = 0;
	for(i = 0; i < data_len; i++){
		sizes += sprintf(cmds + sizes, "%8x ", *pdata);
		pdata++;
	}
	ret = write(fd1, cmds, sizes);
	rsp[0] = 0x7e;
	memcpy(rsp + DIAG_HEADER_LENGTH + 5, &err_rsp, 4);
	memcpy(rsp + 1, buf + 1, DIAG_HEADER_LENGTH + 2);
	rsp_len = DIAG_HEADER_LENGTH + 8 + 2;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;
	close(fd0);
	close(fd1);

	return rsp_len;
}

static int tune_start_read_cfg(char *buf, int len, char *rsp, int rsplen)
{
	MSG_HEAD_T *rsp_head;
	u32 rsp_len = 0;
	u32 sizes = 0;
	u32 extra_len = 0;

	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	parse_lite_r2p0_abc_xml(&(ctx.params.abc));
	parse_lite_r2p0_cms_xml(&(ctx.params.cms));
	parse_lite_r2p0_bld_xml(&(ctx.params.bld));
	parse_lite_r2p0_gamma_xml(&(ctx.params.gamma));
	sizes = sizeof(ctx.params);
	ENG_LOG("%s:  %d\n", __func__, sizes);
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	memcpy(rsp + DIAG_HEADER_LENGTH + 5, (char *)&sizes, sizeof(u32));
	rsp_len = DIAG_HEADER_LENGTH + 10;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;

	return rsp_len;
}

static int tune_midst_read_cfg(char *buf, int len, char *rsp, int rsplen)
{
	char* parambuf;
	u32 *sizes;
	u32 *offset;
	u32 rsp_len = 0;
	short err_rsp = 0;
	int extra_len = 0;
	MSG_HEAD_T *rsp_head;

	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	sizes = (u32 *)(buf + DIAG_HEADER_LENGTH + 5);
	offset = (u32 *)(buf + DIAG_HEADER_LENGTH + 9);
	parambuf = (char *)&(ctx.params);
	parambuf += *offset;
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	memcpy(rsp + DIAG_HEADER_LENGTH + 7, parambuf, *sizes);
	memcpy(rsp + DIAG_HEADER_LENGTH + 5, &err_rsp, 2);
	rsp_len = DIAG_HEADER_LENGTH + 8 + (*sizes);
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;

	return rsp_len;
}

static int tune_end_read_cfg(char *buf, int len, char *rsp, int rsplen)
{
	u32 rsp_len = 0;
	u32 err_rsp = 0;
	MSG_HEAD_T *rsp_head;
	u32 extra_len = 0;

	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	memcpy(rsp + DIAG_HEADER_LENGTH + 5, &err_rsp, 4);
	rsp_len = DIAG_HEADER_LENGTH + 10;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;

	return rsp_len;
}

static int tune_start_write_cfg(char *buf, int len, char *rsp, int rsplen)
{
	u32 *TuneSizes;
	MSG_HEAD_T *rsp_head;
	u32 rsp_len = 0;
	u32 sizes = 0;
	u32 err_rsp = 0;

	TuneSizes = (u32 *)(buf + DIAG_HEADER_LENGTH + 5);
	ctx.offset = 0;
	sizes = sizeof(ctx.params);
	ENG_LOG("%s:  %d  TuneSizes %d\n", __func__, sizes, *TuneSizes);
	if(*TuneSizes != sizes)
		err_rsp	 = 1;
	ENG_LOG("%s:  %d  TuneSizes %d err %d\n", __func__, sizes, *TuneSizes, err_rsp);
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	memcpy(rsp + DIAG_HEADER_LENGTH + 5, &err_rsp, 4);
	rsp_len = DIAG_HEADER_LENGTH + 10;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;

	return rsp_len;
}

static int tune_midst_write_cfg(char *buf, int len, char *rsp, int rsplen)
{
	char *pdata;
	char* parambuf;
	MSG_HEAD_T *rsp_head;
	MSG_HEAD_T *send_head;
	u32 rsp_len = 0;
	u32 sizes;
	u32 err = 0;

	parambuf = (char *)&ctx.params;
	parambuf += ctx.offset;
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	send_head = (MSG_HEAD_T *)(buf + 1);
	sizes = send_head->len - DIAG_HEADER_LENGTH - 4;
	pdata = (char *)(buf + DIAG_HEADER_LENGTH + 5);
	memcpy(parambuf, pdata, sizes);
	ctx.offset += sizes;
	memcpy(rsp ,buf, DIAG_HEADER_LENGTH + 5);
	memcpy(rsp + DIAG_HEADER_LENGTH + 5, &err, 4);
	rsp_len = DIAG_HEADER_LENGTH + 10;
    rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;

	return rsp_len;
}

static int tune_end_write_cfg(char *buf, int len, char *rsp, int rsplen)
{
	MSG_HEAD_T *rsp_head;
	u32 rsp_len = 0;
	u32 err = 0;

	if(ctx.offset == sizeof(ctx.params)) {
		update_lite_r2p0_abc_xml(&ctx.params.abc);
		update_lite_r2p0_cms_xml(&ctx.params.cms);
		update_lite_r2p0_bld_xml(&ctx.params.bld);
		update_lite_r2p0_gamma_xml(&ctx.params.gamma);
		err = 0;
	} else
		err = 1;
	ENG_LOG("%s: offset %d  TuneSizes %d\n", __func__, ctx.offset, err);
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	memcpy(rsp + DIAG_HEADER_LENGTH + 5, &err, 4);
	rsp_len = DIAG_HEADER_LENGTH + 10;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;

	return rsp_len;
}

static bool parse_pq_module_type(u16 cmd){

	switch(cmd) {
		case PQ_VERSION:
		case PQ_GAMMA:
		case PQ_BLD:
		case PQ_CMS:
		case PQ_ABC:
			return true;
		default:
			return false;
	}
}

static int tune_rd_tuning_reg(char *buf, int len, char *rsp, int rsplen)
{

	u32 rsp_len = 0;
	u32 sizes = 0;
	u16 *sub_cmd = 0;
	u8 *pdata;
	int ret = 0;
	int extra_len = 0;
	MSG_HEAD_T *rsp_head;

	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	sub_cmd = (u16 *)(buf + DIAG_HEADER_LENGTH + 5);
	pdata = (u8 *)(rsp + DIAG_HEADER_LENGTH + 7);

	if(!parse_pq_module_type(*sub_cmd))
		goto ERR0;

	switch(*sub_cmd) {
		case PQ_VERSION:
			break;
		case PQ_GAMMA:
			parse_lite_r2p0_gamma_reg(&ctx.params.gamma);
			sizes = sizeof(ctx.params.gamma);
			memcpy(pdata, (u8 *)&ctx.params.gamma, sizes);
			break;
		case PQ_BLD:
			parse_lite_r2p0_bld_reg(&ctx.params.bld);
			sizes = sizeof(ctx.params.bld);
			memcpy(pdata, (u8 *)&ctx.params.bld, sizes);
			break;
		case PQ_CMS:
			parse_lite_r2p0_cms_reg(&ctx.params.cms);
			sizes = sizeof(ctx.params.cms);
			memcpy(pdata, (u8 *)&ctx.params.cms, sizes);
			break;
		case PQ_ABC:
			parse_lite_r2p0_abc_reg(&ctx.params.abc);
			sizes = sizeof(ctx.params.abc);
			memcpy(pdata, (u8 *)&ctx.params.abc, sizes);
			break;
		default:
			break;
	}

ERR0:
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 7);
	rsp_len = DIAG_HEADER_LENGTH + sizes + 8;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len -1] = 0x7e;

	return rsp_len;
}

static int tune_wr_tuning_reg(char *buf, int len, char *rsp, int rsplen)
{

	u32 rsp_len = 0;
	u32 sizes = 0;
	u16 *sub_cmd = 0;
	u8 *pdata;
	u32 *perr;
	u32 err = 0;
	int ret = 0;
	MSG_HEAD_T *rsp_head;

	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	sub_cmd = (u16 *)(buf + DIAG_HEADER_LENGTH + 5);
	pdata = (u8 *)(buf + DIAG_HEADER_LENGTH + 7);
	perr = (u32 *)(rsp + DIAG_HEADER_LENGTH + 5);

	if(!parse_pq_module_type(*sub_cmd)){
		err = 1;
		goto ERR0;
	}

	switch(*sub_cmd) {
		case PQ_VERSION:
			break;
		case PQ_GAMMA:
			sizes = sizeof(ctx.params.gamma);
			memcpy((u8 *)&ctx.params.gamma, pdata, sizes);
			update_lite_r2p0_gamma_reg(&ctx.params.gamma);
			break;
		case PQ_BLD:
			sizes = sizeof(ctx.params.bld);
			memcpy((u8 *)&ctx.params.bld, pdata, sizes);
			update_lite_r2p0_bld_reg(&ctx.params.bld);
			break;
		case PQ_CMS:
			sizes = sizeof(ctx.params.cms);
			memcpy((u8 *)&ctx.params.cms, pdata, sizes);
			update_lite_r2p0_cms_reg(&ctx.params.cms);
			break;
		case PQ_ABC:
			sizes = sizeof(ctx.params.abc);
			memcpy((u8 *)&ctx.params.abc, pdata, sizes);
			update_lite_r2p0_abc_reg(&ctx.params.abc);

			break;
		default:
			break;
	}

ERR0:
	*perr = err;
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	rsp_len = DIAG_HEADER_LENGTH + 10;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len -1] = 0x7e;

	return rsp_len;

}

static int tune_rd_tuning_xml(char *buf, int len, char *rsp, int rsplen)
{

	u32 rsp_len = 0;
	u32 sizes = 0;
	u16* sub_cmd = 0;
	u32 *pdata;
	int ret = 0;
	int err = 0;
	int extra_len = 0;
	MSG_HEAD_T *rsp_head;

	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	sub_cmd = (u16 *)(buf + DIAG_HEADER_LENGTH + 5);
	pdata = (u32 *)(rsp + DIAG_HEADER_LENGTH + 7);

	if(!parse_pq_module_type(*sub_cmd))
		goto ERR0;


	switch(*sub_cmd) {
		case PQ_VERSION:
			break;
		case PQ_GAMMA:
			parse_lite_r2p0_gamma_xml(&ctx.params.gamma);
			sizes = sizeof(ctx.params.gamma);
			memcpy(pdata, (u8 *)&ctx.params.gamma, sizes);
			break;
		case PQ_BLD:
			parse_lite_r2p0_bld_xml(&ctx.params.bld);
			sizes = sizeof(ctx.params.bld);
			memcpy(pdata, (u8 *)&ctx.params.bld, sizes);
			break;
		case PQ_CMS:
			parse_lite_r2p0_cms_xml(&ctx.params.cms);
			sizes = sizeof(ctx.params.cms);
			memcpy(pdata, (u8 *)&ctx.params.cms, sizes);
			break;
		case PQ_ABC:
			parse_lite_r2p0_abc_xml(&ctx.params.abc);
			sizes = sizeof(ctx.params.abc);
			memcpy(pdata, (u8 *)&ctx.params.abc, sizes);
			break;
		default:
			break;
	}

ERR0:
	memcpy(rsp, buf, DIAG_HEADER_LENGTH +  7);
	rsp_len = DIAG_HEADER_LENGTH + sizes + 8;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len -1] = 0x7e;

	return rsp_len;
}

static int tune_wr_tuning_xml(char *buf, int len, char *rsp, int rsplen)
{

	u32 rsp_len = 0;
	u32 sizes = 0;
	u16* sub_cmd = 0;
	u8* pdata;
	u32 *perr;
	u32 err = 0;
	int ret = 0;
	MSG_HEAD_T *rsp_head;

	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	sub_cmd = (u16 *)(buf + DIAG_HEADER_LENGTH + 5);
	pdata = (u8 *)(buf + DIAG_HEADER_LENGTH + 7);
	perr = (u32 *)(rsp + DIAG_HEADER_LENGTH + 5);

	if(!parse_pq_module_type(*sub_cmd)){
		err = 1;
		goto ERR0;
	}

	switch(*sub_cmd) {
		case PQ_VERSION:
			break;
		case PQ_GAMMA:
			sizes = sizeof(ctx.params.gamma);
			memcpy((u8 *)&ctx.params.gamma, pdata, sizes);
			update_lite_r2p0_gamma_xml(&ctx.params.gamma);
			break;
		case PQ_BLD:
			sizes = sizeof(ctx.params.bld);
			memcpy((u8 *)&ctx.params.bld, pdata, sizes);
			update_lite_r2p0_bld_xml(&ctx.params.bld);

			break;
		case PQ_CMS:
			sizes = sizeof(ctx.params.cms);
			memcpy((u8 *)&ctx.params.cms, pdata, sizes);
			update_lite_r2p0_cms_xml(&ctx.params.cms);

			break;
		case PQ_ABC:
			sizes = sizeof(ctx.params.abc);
			memcpy((u8 *)&ctx.params.abc, pdata, sizes);
			update_lite_r2p0_abc_xml(&ctx.params.abc);
			break;
		default:
			break;
	}
ERR0:
	*perr = err;
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	rsp_len = DIAG_HEADER_LENGTH + 10;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len -1] = 0x7e;

	return rsp_len;

}

static int tune_rd_ambient(char *buf, int len, char *rsp, int rsplen)
{
	u32 *pdata = NULL;
	MSG_HEAD_T *rsp_head;
	u32 rsp_len = 0;
	u32 value = 0;
	int ret;
	int fd;
	long long tvalue;
	float *tfpvalue;
	int extra_len = 0;
	char tbuf[256];


	fd = open(SensorLight, O_RDONLY);
	if (fd < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__,
			strerror(errno));
		return errno;
	}

	ret = read(fd, tbuf, sizeof(tbuf));
	if (ret == -1) {
		ENG_LOG("PQ read Ambient Light value Fail\n");
		return -1;
	} else if (ret == 0) {
			 ENG_LOG("PQ read Ambient Light ret 0\n");
	} else {
			ENG_LOG("PQ read %d bytes, buf = %s\n", ret, tbuf);
			tvalue = atoll(tbuf);
			ENG_LOG("PQ tval = %llu\n", tvalue);
			tfpvalue = &tvalue;
			ENG_LOG("PQ float tval = %f\n", *tfpvalue);
			value = (u32)*tfpvalue;
			ENG_LOG("PQ int val = %d\n", value);
	}
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	pdata = (u32 *)(rsp + DIAG_HEADER_LENGTH + 5);
	*pdata = value;
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	rsp_len = DIAG_HEADER_LENGTH + 10;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;

	return rsp_len;
}

static struct tune_ops tune_ops_lite_r2p0 = {
	.tune_connect = tune_connect,
	.tune_rgb_pattern = tune_rgb_pattern,
	.tune_read_regs = tune_read_regs,
	.tune_write_regs = tune_write_regs,
	.tune_start_read_cfg = tune_start_read_cfg,
	.tune_midst_read_cfg = tune_midst_read_cfg,
	.tune_end_read_cfg = tune_end_read_cfg,
	.tune_start_write_cfg = tune_start_write_cfg,
	.tune_midst_write_cfg = tune_midst_write_cfg,
	.tune_end_write_cfg = tune_end_write_cfg,
	.tune_rd_tuning_reg = tune_rd_tuning_reg,
	.tune_wr_tuning_reg = tune_wr_tuning_reg,
	.tune_rd_tuning_xml = tune_rd_tuning_xml,
	.tune_wr_tuning_xml = tune_wr_tuning_xml,
	.tune_rd_ambient = tune_rd_ambient,
};

struct ops_entry tune_lite_r2p0_entry = {
	.version = "dpu-lite-r2p0",
	.ops = &tune_ops_lite_r2p0,
};

