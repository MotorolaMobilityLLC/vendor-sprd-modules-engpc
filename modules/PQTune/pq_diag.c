#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <ctype.h>
#include <sys/reboot.h>
#include <utils/Log.h>
#include "sprd_fts_type.h"
#include "sprd_fts_log.h"
#include "sprd_fts_list.h"
#include "pq_diag.h"
#include "pq_sysfs.h"

LIST_HEAD(pq_tune_head);

static struct tune_ops *diag_ops = NULL;


static int pq_cmd_connect(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops) {
		ENG_LOG("pq_cmd_connect enter");
		rsp_len = diag_ops->tune_connect(buf, len, rsp, rsplen);
	} else
		ENG_LOG("pq_cmd_connect fail");

	return rsp_len;
}


static int pq_cmd_rgb_pattern(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_rgb_pattern(buf, len, rsp, rsplen);

	return rsp_len;
}

static int pq_cmd_read_regs(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_read_regs(buf, len, rsp, rsplen);

	return rsp_len;
}

static int pq_cmd_write_regs(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_write_regs(buf, len, rsp, rsplen);

	return rsp_len;
}

static int pq_cmd_start_read_cfg(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_start_read_cfg(buf, len, rsp, rsplen);

	return rsp_len;
}

static int pq_cmd_midst_read_cfg(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_midst_read_cfg(buf, len, rsp, rsplen);

	return rsp_len;
}

static int pq_cmd_end_read_cfg(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_end_read_cfg(buf, len, rsp, rsplen);

	return rsp_len;
}

static int pq_cmd_start_write_cfg(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_start_write_cfg(buf, len, rsp, rsplen);

	return rsp_len;
}

static int pq_cmd_midst_write_cfg(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_midst_write_cfg(buf, len, rsp, rsplen);

	return rsp_len;
}

static int pq_cmd_end_write_cfg(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_end_write_cfg(buf, len, rsp, rsplen);

	return rsp_len;
}

static int pq_cmd_rd_tuning_reg(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_rd_tuning_reg(buf, len, rsp, rsplen);

	return rsp_len;
}

static int pq_cmd_wr_tuning_reg(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_wr_tuning_reg(buf, len, rsp, rsplen);

	return rsp_len;
}

int pq_cmd_rd_tuning_xml(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_rd_tuning_xml(buf, len, rsp, rsplen);

	return rsp_len;
}

int pq_cmd_wr_tuning_xml(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_wr_tuning_xml(buf, len, rsp, rsplen);

	return rsp_len;
}

int pq_cmd_rd_ambient(char *buf, int len, char *rsp, int rsplen)
{
	int rsp_len;

	if (diag_ops)
		rsp_len = diag_ops->tune_rd_ambient(buf, len, rsp, rsplen);

	return rsp_len;
}

int pq_adapt_dpu_core(void)
{
	int fd;
	int ret;
	char version[20] = {0};
	
	fd = open(DpuVersion, O_RDONLY);
	if (fd < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__,
			strerror(errno));
		return errno;
	}
	ret = read(fd, version, sizeof(version));
	diag_ops = tune_ops_attach(version, &pq_tune_head);

	if (diag_ops)
		return 0;
	else
		return -1;
}

void register_this_module_ext(struct eng_callback *reg, int *num) {
  int moudles_num = 0;
  int ret;
  struct eng_callback *tmp = reg;
  ENG_LOG("register_this_module_ext libPQTune");

  pq_tune_ops_register(tune_lite_r2p0_entry);
  pq_tune_ops_register(tune_r2p0_entry);
  ret = pq_adapt_dpu_core();
  if (ret) {
  	ENG_LOG("PQ adapt dpu ip fail");
	//diag_ops = dpu_r2p0_list.ops;
	return;
	
  }
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


