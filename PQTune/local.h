#ifndef __PQ_LOCAL_H__
#define __PQ_LOCAL_H__

#include "../engopt.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define HSV_EN         (1 << 2)
#define CMS_EN         (1 << 3)
#define SLP_EN         (1 << 4)
#define GAMMA_EN       (1 << 5)

#define DispcBg 			"/sys/class/display/dispc0/bg_color"
#define PanelSize 			"/sys/class/display/panel0/resolution"
#define DispcWrRegs			"/sys/class/display/dispc0/wr_regs"
#define DispcRegsOffset		"/sys/class/display/dispc0/regs_offset"
#define SensorLight			"sys/devices/virtual/sprd_sensorhub/sensor_hub/iio/in_illuminance_raw"

#define PQEnable			"/sys/class/display/dispc0/PQ/enable"
#define PQDisable			"/sys/class/display/dispc0/PQ/disable"
#define PQStatus			"/sys/class/display/dispc0/PQ/status"
#define DpuGamma			"/sys/class/display/dispc0/PQ/gamma"
#define DpuSlp				"/sys/class/display/dispc0/PQ/slp"
#define DpuCm				"/sys/class/display/dispc0/PQ/cm"
#define DpuHsv				"/sys/class/display/dispc0/PQ/hsv"
#define FlipDisable			"/sys/class/display/dispc0/disable_flip"

#define bld_xml				"/mnt/vendor/enhance/bld.xml"
#define abc_xml				"/mnt/vendor/enhance/abc.xml"
#define gamma_xml			"/mnt/vendor/enhance/gamma.xml"
#define cms_xml				"/mnt/vendor/enhance/cms.xml"

#define ChipInfo			"/proc/cmdline"
#define Brightness			"/sys/class/backlight/sprd_backlight/brightness"

#define DispState			"/sys/class/adf/sprd-adf-dev-interface0/dpms_state"
#define DispOn				0
#define DispOff				3

#endif

