#ifndef __PQ_TUNE_PARAM_H__
#define __PQ_TUNE_PARAM_H__


#include "local.h"

struct subversion
{
    u32 version; // reserved
    u32 enable;  // 0/1
};

struct mainversion
{
    u32 version;
    u32 modify_time; // YYMMDDHHMM
};

struct gamma_lut
{
    u16 r[256];
    u16 g[256];
    u16 b[256];
};

struct hsv_lut_table
{
    u16 hue;
    u16 sat;
};

struct cm_cfg
{
    short coef00;
    short coef01;
    short coef02;
    short coef03;
    short coef10;
    short coef11;
    short coef12;
    short coef13;
    short coef20;
    short coef21;
    short coef22;
    short coef23;
};

struct hsv_lut
{
    struct hsv_lut_table table[360];
};

struct hsv_cm
{
    struct hsv_lut hsv;
    struct cm_cfg cm;
};

struct rgb_cm_mapping
{
    u8 rgb;
    u8 cmindex;
};


struct slp_cfg
{
    u8 brightness;
    u8 conversion_matrix;
    u8 brightness_step;
    u8 second_bright_factor;
    u8 first_percent_th;
    u8 first_max_bright_th;
};

struct slp_mapping_table_item
{
    u16 ambient;
    u16 slp_brightness_factor;
};

struct slp_mapping_table
{
	u32 map_num;
    struct slp_mapping_table_item item[128];
};

struct bl_mapping_table_item
{
    u16 ambient;
    u16 backlight;
};

struct bl_mapping_table
{
	u32 map_num;
    struct bl_mapping_table_item item[256];
};

struct gamma_common
{
    struct subversion version;
    struct gamma_lut gamma;
};

struct bld_common
{
    struct subversion version;
    u32 mode; // 1:low; 2:standard; 3:high
    struct hsv_cm hsvcm[3];
};

struct cms_common
{
    struct subversion version;
    u32 mode;
    struct hsv_cm hsvcm[3];
    struct rgb_cm_mapping rgbcm[10];
	u16 cm_mode;   // 1:cold; 2:warm; 3-12:rgb auto
    struct cm_cfg cm[12];
};

struct abc_common
{
    struct subversion version;
    struct slp_cfg  cfg;
    u8 mode;                // 1:low; 2:normal
    u8 slpbl_mode;          // 1:slp; 2:bl
    u32	backlight;;
    struct slp_mapping_table slp_table[2];
    struct bl_mapping_table bl_table[2];
};

struct pq_tuning_parm
{
    struct mainversion version;
    struct gamma_common gamma;
    struct bld_common bld;
    struct cms_common cms;
    struct abc_common abc;
};

struct ambient_light
{
    u32 light;
};

struct	PQContext
{
	struct pq_tuning_parm PQParam;
	u32 offset;
};
#endif
