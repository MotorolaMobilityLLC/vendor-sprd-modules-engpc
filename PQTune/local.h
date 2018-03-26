#ifndef __PQ_LOCAL_H__
#define __PQ_LOCAL_H__

#include "../engopt.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define DispcBg 			"/sys/class/display/dispc0/bg_color"
#define PanelSize 			"/sys/class/display/panel0/resolution"
#define DispcWrRegs			"/sys/class/display/dispc0/wr_regs"
#define DispcRegsOffset		"/sys/class/display/dispc0/regs_offset"
#define SensorLight			"sys/devices/virtual/sprd_sensorhub/sensor_hub/iio/in_illuminance_raw"

#define PQEnable			"/sys/class/display/dispc0/PQ/enable"
#define PQDisable			"/sys/class/display/dispc0/PQ/disable"
#define DpuGamma			"/sys/class/display/dispc0/PQ/gamma"
#define DpuSlp				"/sys/class/display/dispc0/PQ/slp"
#define DpuCm				"/sys/class/display/dispc0/PQ/cm"
#define DpuHsv				"/sys/class/display/dispc0/PQ/hsv"

#define bld_xml				"/productinfo/enhance/bld.xml"
#define abc_xml				"/productinfo/enhance/abc.xml"
#define gamma_xml			"/productinfo/enhance/gamma.xml"
#define cms_xml				"/productinfo/enhance/cms.xml"

#endif

