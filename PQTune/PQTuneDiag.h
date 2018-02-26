#ifndef __PQ_DIAG_H__
#define __PQ_DIAG_H__

#include "PQTuningParmDefine.h"

#define MAX_NAME_LEN 64
#define MAX_OPEN_TIMES 100

struct	PQContext PQCtx;

typedef struct _RESOLUTION_T
{
  unsigned int HPixel;
  unsigned int VPixel;
}RESOLUTION_T;

typedef struct _DUT_INFO_T
{
  unsigned int nSize;             // size of this struct itself
  unsigned int nVersion;          // the version of struct itself
  char szModelName[MAX_NAME_LEN]; // Model name,such as SC7731.
  char szChipName[MAX_NAME_LEN];  // Chip model ,such as 7731
  RESOLUTION_T stResolution;      // Resolution
  float  fLcdInch;                // LcdInch
}DUT_INFO_T;

typedef enum {
  CMD_PQ_DUMMY,
  CMD_PQ_CONNECT,
  CMD_PQ_RGB_PATTERN,
  CMD_PQ_RD_REGS,
  CMD_PQ_WR_REGS,
  CMD_PQ_START_READ_CFG,
  CMD_PQ_MIDST_READ_CFG,
  CMD_PQ_END_READ_CFG,
  CMD_PQ_START_WRITE_CFG,
  CMD_PQ_MIDST_WRITE_CFG,
  CMD_PQ_END_WRITE_CFG,
  CMD_PQ_RD_TUNING_REG,
  CMD_PQ_WR_TUNING_REG,
  CMD_PQ_RD_TUNING_XML,
  CMD_PQ_WR_TUNING_XML,
  CMD_PQ_RD_AMBIENT,
  CMD_PQ_END
} DIAG_PQ_CMD_TYPE;

typedef enum {
  PQ_VERSION,
  PQ_GAMMA,
  PQ_BLD,
  PQ_CMS,
  PQ_ABC,
  E_UNDEFINE,
} PQ_MODULE_TYPE;

int pq_cmd_rgb_pattern(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_connect(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_read_regs(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_write_regs(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_start_read_cfg(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_midst_read_cfg(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_end_read_cfg(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_start_write_cfg(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_midst_write_cfg(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_end_write_cfg(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_rd_tuning_reg(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_wr_tuning_reg(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_rd_tuning_xml(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_wr_tuning_xml(char *buf, int len, char *rsp, int rsplen);
int pq_cmd_rd_ambient(char *buf, int len, char *rsp, int rsplen);
int parse_abc_xml(struct abc_common *abc);
int update_abc_xml(struct abc_common *abc);
int parse_bld_xml(struct bld_common *bld);
int update_bld_xml(struct bld_common *bld);
int parse_cms_xml(struct cms_common *cms);
int update_cms_xml(struct cms_common *cms);
int update_gamma_xml(struct gamma_common *gamma);
int parse_gamma_xml(struct gamma_common *gamma);
int parse_gamma_reg(struct gamma_common *gamma);
int update_gamma_reg(struct gamma_common *gamma);
int parse_abc_reg(struct abc_common *abc);
int update_abc_reg(struct abc_common *abc);
int parse_cms_reg(struct cms_common *cms);
int update_cms_reg(struct cms_common *cms);
int parse_bld_reg(struct bld_common *bld);
int update_bld_reg(struct bld_common *bld);


#endif
