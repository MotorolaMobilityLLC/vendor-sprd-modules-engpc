#ifndef __ENGPC_ADAPTER_H__
#define __ENGPC_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif


void usb_vser_enable(int mode); //0: autotest, 1: cali
const char* usb_getEnablePath();
const char* usb_getMaxSpeedPath();
const char* usb_getStatePath();
int usb_getConf(char* buff, int nlen);

const char* getChargeStopPath();
int is_data_dir_ready();

#ifdef __cplusplus
}
#endif

#endif




