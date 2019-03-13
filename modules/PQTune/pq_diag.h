#define MAX_NAME_LEN 64
#define MAX_OPEN_TIMES 100


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

struct tune_ops {
	int (*tune_connect)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_rgb_pattern)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_read_regs)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_write_regs)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_start_read_cfg)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_midst_read_cfg)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_end_read_cfg)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_start_write_cfg)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_midst_write_cfg)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_end_write_cfg)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_rd_tuning_reg)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_wr_tuning_reg)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_rd_tuning_xml)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_wr_tuning_xml)(char *buf, int len, char *rsp, int rsplen);
	int (*tune_rd_ambient)(char *buf, int len, char *rsp, int rsplen);
};

extern struct list_head pq_tune_head;
extern struct ops_entry tune_lite_r2p0_entry;
extern struct ops_entry tune_r2p0_entry;
extern int tune_ops_register(struct ops_entry *entry, struct list_head *head);
extern void *tune_ops_attach(const char *str, struct list_head *head);
#define pq_tune_ops_register(entry) \
	            tune_ops_register(&entry, &pq_tune_head)

