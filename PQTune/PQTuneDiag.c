#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <cutils/android_reboot.h>
#include <cutils/sockets.h>
#include <poll.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "PQTuneDiag.h"
#include "Ambient.h"
#include <ctype.h>
#include <sys/reboot.h>
#include <utils/Log.h>
#include "../vlog.h"
#include "../eng_diag.h"
#include "../eng_pcclient.h"
#include "../engopt.h"
#include "../eng_modules.h"

void HexStrstoInt(char *buf, int size)
{
 	int *temp = (int*) buf;
	u32 i = 0;
	u32 j = 0;
	for(; i < size; i++){
		temp[i] = strtol(buf + j, NULL, 16);
		j += 9;
	}
}

int eng_diag_encode7d7e(char *buf, int len, int *extra_len) {
  int i, j;
  char tmp;

  ENG_LOG("%s: len=%d", __FUNCTION__, len);

  for (i = 0; i < len; i++) {
    if ((buf[i] == 0x7d) || (buf[i] == 0x7e)) {
      tmp = buf[i] ^ 0x20;
      ENG_LOG("%s: tmp=%x, buf[%d]=%x", __FUNCTION__, tmp, i, buf[i]);
      buf[i] = 0x7d;
      for (j = len; j > i + 1; j--) {
        buf[j] = buf[j - 1];
      }
      buf[i + 1] = tmp;
      len++;
      (*extra_len)++;

      ENG_LOG("%s: AFTER:[%d]", __FUNCTION__, len);
      for (j = 0; j < len; j++) {
        ENG_LOG("%x,", buf[j]);
      }
    }
  }

  return len;
}

int MultTimes(int base, int n)
{
	int mt = 1;
	int m;

	for(m = 0; m < n; m++)
		mt *= base;
	return mt;
}

void parse_panelsize(char *data,u32 count,DUT_INFO_T  *dut_info_t)
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

	ENG_LOG("PQ %d %d \n",dut_info_t->stResolution.HPixel,  dut_info_t->stResolution.VPixel);
}


int pq_cmd_connect(char *buf, int len, char *rsp, int rsplen)
{
	DUT_INFO_T  *dut_info;
	MSG_HEAD_T *rsp_head;
	u32 rsp_len = 0;
	char info[1024] = {0};
	u32 sizes = 0;
	int fd;
	int fd1;
	char *pchar;
	char *ptemp;

	fd = open(PanelSize, O_RDONLY);
	fd1 = open(ChipInfo, O_RDONLY);
	if (fd < 0 || fd1 < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__,
			strerror(errno));
		return errno;
	}

	dut_info =  (DUT_INFO_T  *)malloc(sizeof(DUT_INFO_T));
	if(!dut_info) {
		ENG_LOG("PQ dut info alloc fail\n");
		return -1;
	}
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	memset(dut_info, 0, sizeof(DUT_INFO_T));
	sizes = read(fd1, info, 1024);
	pchar = strstr(info, "androidboot.hardware");
	pchar = strstr(pchar, "=");
	ptemp = strchr(pchar + 1, ' ');
	sizes = ptemp - pchar - 1;
	strncpy(dut_info->szModelName, pchar + 1, sizes);
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
	ENG_LOG("pq_cmd_connect sucess\n");
	return rsp_len;
}


int pq_cmd_rgb_pattern(char *buf, int len, char *rsp, int rsplen)
{
	int fd;
	int ret = 0;
	char rgb[10] = {0};
	bool exit = false;
	u32 *pdata = NULL;
	u32 *rsp_pdata = NULL;
	MSG_HEAD_T *rsp_head;
	u32 rsp_len = 0;
	u32 extra_len = 0;

	fd= open(DispcBg, O_RDWR);
	if (fd < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}

	system("stop");
	pdata = (u32 *)(buf + DIAG_HEADER_LENGTH + 5);
	rsp_pdata = (u32 *)(rsp + DIAG_HEADER_LENGTH + 5);;
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	if(*pdata != 0xffffffff){
		sprintf(rgb, "%x", *pdata);
		ret = write(fd, rgb, sizeof(rgb));
		exit = false;
	}else {
		ret = 0;
		exit = true;
	}
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	if(ret == -1)
		*rsp_pdata = 1;
	else
		*rsp_pdata = 0;
	rsp_len = DIAG_HEADER_LENGTH + 10;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;
	close(fd);
	if(exit)
		system("start");
	return rsp_len;

}

int pq_cmd_read_regs(char *buf, int len, char *rsp, int rsplen)
{
	int fd0;
	int fd1;
	u32 *pdata;
	u32 offset;
	u32 length;
	u32 sizes = 0;
	u32 rsp_len = 0;
	char *cmds[30];
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
	bufs = (char *)(rsp + DIAG_HEADER_LENGTH + 13);
	sizes = sprintf(cmds, "%x ", offset);
	sprintf(cmds + sizes, "%x ", length);
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

int pq_cmd_write_regs(char *buf, int len, char *rsp, int rsplen)
{
	int fd0;
	int fd1;
	u32 *pcmd;
	int offset;
	int length;
	u32 rsp_len = 0;
	char* cmds;
	int sizes;
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
	sizes = sprintf(cmds + sizes, "%8x ", offset);
	sizes += sprintf(cmds + sizes, "%8x ", length);
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	send_head = (MSG_HEAD_T *)(buf + 1);
	data_len = send_head->len - 12 - DIAG_HEADER_LENGTH;
	pdata = (int*)(buf + DIAG_HEADER_LENGTH + 13);
	ret = write(fd0 ,cmds, sizeof(cmds));
	sizes = 0;
	for(i = 0; i < data_len / 4; i++){
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

int pq_cmd_start_read_cfg(char *buf, int len, char *rsp, int rsplen)
{
	MSG_HEAD_T *rsp_head;
	u32 rsp_len = 0;
	u32 sizes = 0;
	u32 extra_len = 0;

	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	parse_abc_xml(&(PQCtx.PQParam.abc));
	parse_cms_xml(&(PQCtx.PQParam.cms));
	parse_bld_xml(&(PQCtx.PQParam.bld));
	parse_gamma_xml(&(PQCtx.PQParam.gamma));
	sizes = sizeof(struct pq_tuning_parm);
	ENG_LOG("%s:  %d\n", __func__, sizes);
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	memcpy(rsp + DIAG_HEADER_LENGTH + 5, (char *)&sizes, sizeof(u32));
	rsp_len = DIAG_HEADER_LENGTH + 10;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;

	return rsp_len;
}

int pq_cmd_midst_read_cfg(char *buf, int len, char *rsp, int rsplen)
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
	parambuf = (char *)&(PQCtx.PQParam);
	parambuf += *offset;
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	memcpy(rsp + DIAG_HEADER_LENGTH + 7, parambuf, *sizes);
	memcpy(rsp + DIAG_HEADER_LENGTH + 5, &err_rsp, 2);
	rsp_len = DIAG_HEADER_LENGTH + 8 + (*sizes);
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;

	return rsp_len;
}

int pq_cmd_end_read_cfg(char *buf, int len, char *rsp, int rsplen)
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

int pq_cmd_start_write_cfg(char *buf, int len, char *rsp, int rsplen)
{
	u32 *TuneSizes;
	MSG_HEAD_T *rsp_head;
	u32 rsp_len = 0;
	u32 sizes = 0;
	u32 err_rsp = 0;

	TuneSizes = (u32 *)(buf + DIAG_HEADER_LENGTH + 5);
	PQCtx.offset = 0;
	sizes = sizeof(PQCtx.PQParam);
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

int pq_cmd_midst_write_cfg(char *buf, int len, char *rsp, int rsplen)
{
	char *pdata;
	char* parambuf;
	MSG_HEAD_T *rsp_head;
	MSG_HEAD_T *send_head;
	u32 rsp_len = 0;
	u32 sizes;
	u32 err = 0;

	parambuf = (char *)&PQCtx.PQParam;
	parambuf += PQCtx.offset;
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	send_head = (MSG_HEAD_T *)(buf + 1);
	sizes = send_head->len - DIAG_HEADER_LENGTH - 4;
	pdata = (char *)(buf + DIAG_HEADER_LENGTH + 5);
	memcpy(parambuf, pdata, sizes);
	PQCtx.offset += sizes;
	memcpy(rsp ,buf, DIAG_HEADER_LENGTH + 5);
	memcpy(rsp + DIAG_HEADER_LENGTH + 5, &err, 4);
	rsp_len = DIAG_HEADER_LENGTH + 10;
    rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;

	return rsp_len;
}

int pq_cmd_end_write_cfg(char *buf, int len, char *rsp, int rsplen)
{
	MSG_HEAD_T *rsp_head;
	u32 rsp_len = 0;
	u32 err = 0;

	if(PQCtx.offset == sizeof(PQCtx.PQParam)) {
		update_abc_xml(&PQCtx.PQParam.abc);
		update_cms_xml(&PQCtx.PQParam.cms);
		update_bld_xml(&PQCtx.PQParam.bld);
		update_gamma_xml(&PQCtx.PQParam.gamma);
		err = 0;
	} else
		err = 1;
	ENG_LOG("%s: offset %d  TuneSizes %d\n", __func__, PQCtx.offset, err);
	rsp_head = (MSG_HEAD_T *)(rsp + 1);
	memcpy(rsp, buf, DIAG_HEADER_LENGTH + 5);
	memcpy(rsp + DIAG_HEADER_LENGTH + 5, &err, 4);
	rsp_len = DIAG_HEADER_LENGTH + 10;
	rsp_head->len = rsp_len - 2;
	rsp[rsp_len - 1] = 0x7e;

	return rsp_len;
}

bool parse_pq_module_type(u16 cmd){

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

int pq_cmd_rd_tuning_reg(char *buf, int len, char *rsp, int rsplen)
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
			parse_gamma_reg(&PQCtx.PQParam.gamma);
			sizes = sizeof(PQCtx.PQParam.gamma);
			memcpy(pdata, (u8 *)&PQCtx.PQParam.gamma, sizes);
			break;
		case PQ_BLD:
			parse_bld_reg(&PQCtx.PQParam.bld);
			sizes = sizeof(PQCtx.PQParam.bld);
			memcpy(pdata, (u8 *)&PQCtx.PQParam.bld, sizes);
			break;
		case PQ_CMS:
			parse_cms_reg(&PQCtx.PQParam.cms);
			sizes = sizeof(PQCtx.PQParam.cms);
			memcpy(pdata, (u8 *)&PQCtx.PQParam.cms, sizes);
			break;
		case PQ_ABC:
			parse_abc_reg(&PQCtx.PQParam.abc);
			sizes = sizeof(PQCtx.PQParam.abc);
			memcpy(pdata, (u8 *)&PQCtx.PQParam.abc, sizes);
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

int pq_cmd_wr_tuning_reg(char *buf, int len, char *rsp, int rsplen)
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
			sizes = sizeof(PQCtx.PQParam.gamma);
			memcpy((u8 *)&PQCtx.PQParam.gamma, pdata, sizes);
			update_gamma_reg(&PQCtx.PQParam.gamma);
			break;
		case PQ_BLD:
			sizes = sizeof(PQCtx.PQParam.bld);
			memcpy((u8 *)&PQCtx.PQParam.bld, pdata, sizes);
			update_bld_reg(&PQCtx.PQParam.bld);
			break;
		case PQ_CMS:
			sizes = sizeof(PQCtx.PQParam.cms);
			memcpy((u8 *)&PQCtx.PQParam.cms, pdata, sizes);
			update_cms_reg(&PQCtx.PQParam.cms);
			break;
		case PQ_ABC:
			sizes = sizeof(PQCtx.PQParam.abc);
			memcpy((u8 *)&PQCtx.PQParam.abc, pdata, sizes);
			update_abc_reg(&PQCtx.PQParam.abc);

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

int pq_cmd_rd_tuning_xml(char *buf, int len, char *rsp, int rsplen)
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
			parse_gamma_xml(&PQCtx.PQParam.gamma);
			sizes = sizeof(PQCtx.PQParam.gamma);
			memcpy(pdata, (u8 *)&PQCtx.PQParam.gamma, sizes);
			break;
		case PQ_BLD:
			parse_bld_xml(&PQCtx.PQParam.bld);
			sizes = sizeof(PQCtx.PQParam.bld);
			memcpy(pdata, (u8 *)&PQCtx.PQParam.bld, sizes);
			break;
		case PQ_CMS:
			parse_cms_xml(&PQCtx.PQParam.cms);
			sizes = sizeof(PQCtx.PQParam.cms);
			memcpy(pdata, (u8 *)&PQCtx.PQParam.cms, sizes);
			break;
		case PQ_ABC:
			parse_abc_xml(&PQCtx.PQParam.abc);
			sizes = sizeof(PQCtx.PQParam.abc);
			memcpy(pdata, (u8 *)&PQCtx.PQParam.abc, sizes);
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

int pq_cmd_wr_tuning_xml(char *buf, int len, char *rsp, int rsplen)
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
			sizes = sizeof(PQCtx.PQParam.gamma);
			memcpy((u8 *)&PQCtx.PQParam.gamma, pdata, sizes);
			update_gamma_xml(&PQCtx.PQParam.gamma);
			break;
		case PQ_BLD:
			sizes = sizeof(PQCtx.PQParam.bld);
			memcpy((u8 *)&PQCtx.PQParam.bld, pdata, sizes);
			update_bld_xml(&PQCtx.PQParam.bld);

			break;
		case PQ_CMS:
			sizes = sizeof(PQCtx.PQParam.cms);
			memcpy((u8 *)&PQCtx.PQParam.cms, pdata, sizes);
			update_cms_xml(&PQCtx.PQParam.cms);

			break;
		case PQ_ABC:
			sizes = sizeof(PQCtx.PQParam.abc);
			memcpy((u8 *)&PQCtx.PQParam.abc, pdata, sizes);
			update_abc_xml(&PQCtx.PQParam.abc);
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

int pq_cmd_rd_ambient(char *buf, int len, char *rsp, int rsplen)
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

void register_this_module_ext(struct eng_callback *reg, int *num) {
  int moudles_num = 0;
  struct eng_callback *tmp = reg;
  ENG_LOG("register_this_module_ext libPQTune");

  /***PQ Connect Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x01; // sub cmd
  tmp->diag_ap_cmd = 0x1;
  tmp->eng_diag_func = pq_cmd_connect; // rsp function ptr
  moudles_num++;
  tmp++;

  /***PQ Gamma Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0x2;
  tmp->eng_diag_func = pq_cmd_rgb_pattern; // rsp function ptr
  moudles_num++;
  tmp++;

   /***PQ Read Regs Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0x3;
  tmp->eng_diag_func = pq_cmd_read_regs; // rsp function ptr
  moudles_num++;
  tmp++;

   /***PQ Write Regs Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0x4;
  tmp->eng_diag_func = pq_cmd_write_regs; // rsp function ptr
  moudles_num++;
  tmp++;

    /***PQ cfg read Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0x5;
  tmp->eng_diag_func = pq_cmd_start_read_cfg; // rsp function ptr
  moudles_num++;
  tmp++;
     /***PQ cfg read Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0x6;
  tmp->eng_diag_func = pq_cmd_midst_read_cfg; // rsp function ptr
  moudles_num++;
  tmp++;

    /***PQ cfg read Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0x7;
  tmp->eng_diag_func = pq_cmd_end_read_cfg; // rsp function ptr
  moudles_num++;
  tmp++;

    /***PQ cfg write Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0x8;
  tmp->eng_diag_func = pq_cmd_start_write_cfg; // rsp function ptr
  moudles_num++;
  tmp++;

    /***PQ cfg write Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0x9;
  tmp->eng_diag_func = pq_cmd_midst_write_cfg; // rsp function ptr
  moudles_num++;
  tmp++;

   /***PQ Connect Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0xa;
  tmp->eng_diag_func = pq_cmd_end_write_cfg; // rsp function ptr
  moudles_num++;
  tmp++;

    /***PQ Read Tuning regs Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0xb;
  tmp->eng_diag_func = pq_cmd_rd_tuning_reg; // rsp function ptr
  moudles_num++;
  tmp++;

    /***PQ Write Tunning regs Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0xc;
  tmp->eng_diag_func = pq_cmd_wr_tuning_reg; // rsp function ptr
  moudles_num++;
  tmp++;

    /***PQ Tuning xml Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0xd;
  tmp->eng_diag_func = pq_cmd_rd_tuning_xml; // rsp function ptr
  moudles_num++;
  tmp++;

   /***PQ Tuning xml Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0xe;
  tmp->eng_diag_func = pq_cmd_wr_tuning_xml; // rsp function ptr
  moudles_num++;
  tmp++;

    /***PQ Read Ambient Cmd***/
  tmp->type = 0x5D;   // main cmd
  tmp->subtype = 0x1; // sub cmd
  tmp->diag_ap_cmd = 0xf;
  tmp->eng_diag_func = pq_cmd_rd_ambient; // rsp function ptr
  moudles_num++;

  *num = moudles_num;

  ENG_LOG("register_this_module_ext: %d - %d", *num, moudles_num);
}


