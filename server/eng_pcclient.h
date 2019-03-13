#ifndef __ENG_PCCLIENT_H__
#define __ENG_PCCLIENT_H__

#define ENG_CALISTR "calibration"
#define ENG_TESTMODE "engtestmode"
#define ENG_STREND "\r\n"
#define ENG_USBIN "/sys/class/android_usb/android0/state"
#define ENG_USBCONNECTD "CONFIGURED"
#define ENG_FACOTRYMODE_FILE "/mnt/vendor/factorymode.file"
#define ENG_FACOTRYSYNC_FILE "/factorysync.file"
#define ENG_WIFI_CONFIG_FILE "/mnt/vendor/2351_connectivity_configure.ini"
#define ENG_IMEI1_CONFIG_FILE "/mnt/vendor/imei1.txt"
#define ENG_IMEI2_CONFIG_FILE "/mnt/vendor/imei2.txt"
#define ENG_IMEI3_CONFIG_FILE "/mnt/vendor/imei3.txt"
#define ENG_IMEI4_CONFIG_FILE "/mnt/vendor/imei4.txt"
#define ENG_MODEMRESET_PROPERTY "persist.sys.sprd.modemreset"
#define ENG_USB_PROPERTY "persist.sys.sprd.usbfactorymode"
#define RAWDATA_PROPERTY "sys.rawdata.ready"
#define PROP_MODEM_LOG_DEST    "persist.vendor.modem.log_dest"
#define PROP_WCN_LOG_DEST      "persist.vendor.wcn.log_dest"
#define PROP_MODEM_SUPPORT     "ro.vendor.modem.support"

#define PROP_AGDSP_LOG "ro.vendor.ag.log"
#define PROP_AGDSP_PCM "ro.vendor.ag.pcm"
#define PROP_AGDSP_MEM "ro.vendor.ag.mem"

#define ENG_ATDIAG_AT "AT+SPBTWIFICALI="
#define ENG_BUFFER_SIZE 2048
#define ENG_CMDLINE_LEN 1024
#define ENG_DEV_PATH_LEN 260

enum { ENG_CMDERROR = -1, ENG_CMD4LINUX = 0, ENG_CMD4MODEM };

enum {
  ENG_RUN_TYPE_WCDMA = 0,
  ENG_RUN_TYPE_TD,
  ENG_RUN_TYPE_WCN,
  ENG_RUN_TYPE_LTE,
  ENG_RUN_TYPE_MAX
};

typedef struct eng_host_int {
  char dev_at[ENG_DEV_PATH_LEN];
  char dev_diag[ENG_DEV_PATH_LEN];
  char dev_log[ENG_DEV_PATH_LEN];
  int dev_type;
  int cali_flag;
} eng_host_int_t;

typedef struct eng_modem_int {
  char at_chan[ENG_DEV_PATH_LEN];
  char diag_chan[ENG_DEV_PATH_LEN];
  char log_chan[ENG_DEV_PATH_LEN];
} eng_modem_int_t;

typedef struct eng_dev_info {
  eng_host_int_t host_int;
  eng_modem_int_t modem_int;
} eng_dev_info_t;

#endif