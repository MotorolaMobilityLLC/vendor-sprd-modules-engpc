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
#include "PQTuningParmDefine.h"
#include "PQTuneCore.h"
#include "PQDiag.h"
#include "PQSysfs.h"
#include "Minui/minui.h"

PQTuneCoreLiteR2p0::PQTuneCoreLiteR2p0(int ver):PQTuneCore(DPU_LITE_R2P0)
{
	if (ver != DPU_LITE_R2P0)
		return;
	gamma = new GammaParserLiteR2p0(),
	bld = new BldParserLiteR2p0();
	cms = new CmsParserLiteR2p0();
	abc = new AbcParserLiteR2p0();
	ctx = (uint08_t *)malloc(sizeof(pq_tuning_parm_sharkl5));
	tune_sizes = sizeof(pq_tuning_parm_sharkl5);
	gamma_size = sizeof(gamma_common_sharkl5);
	bld_size = sizeof(bld_common_sharkl5);
	cms_size = sizeof(cms_common_sharkl5);
	abc_size = sizeof(abc_common_sharkl5);
	offset = 0;
	version = ver;
}

PQTuneCoreR3p0::PQTuneCoreR3p0(int ver):PQTuneCoreLiteR2p0(DPU_R3P0) 
{
	if (ver != DPU_R3P0)
		return;
	gamma = new GammaParserLiteR2p0(),
	bld = new BldParserLiteR2p0();
	cms = new CmsParserLiteR2p0();
	abc = new AbcParserR3p0();
	ctx = (uint08_t *)malloc(sizeof(pq_tuning_parm_roc1));
	tune_sizes = sizeof(pq_tuning_parm_roc1);
	gamma_size = sizeof(gamma_common_sharkl5);
	bld_size = sizeof(bld_common_sharkl5);
	cms_size = sizeof(cms_common_sharkl5);
	abc_size = sizeof(abc_common_roc1);
	offset = 0;
	version = ver;
}

PQTuneCoreR4p0::PQTuneCoreR4p0(int ver):PQTuneCoreLiteR2p0(DPU_R4P0) 
{
	if (ver != DPU_R4P0)
		return;
	gamma = new GammaParserLiteR2p0(),
	bld = new BldParserLiteR2p0();
	cms = new CmsParserR4p0();
	abc = new AbcParserR4p0();
	hsv = new HsvParser();
	ctx = (uint08_t *)malloc(sizeof(pq_tuning_parm_sharkl5Pro));
	tune_sizes = sizeof(pq_tuning_parm_sharkl5Pro);
	gamma_size = sizeof(gamma_common_sharkl5);
	bld_size = sizeof(bld_common_sharkl5);
	cms_size = sizeof(cms_common_sharkl5Pro);
	abc_size = sizeof(abc_common_sharkl5Pro);
	hsv_size = sizeof(hsv_common_sharkl5Pro);
	offset = 0;
	version = ver;
}

PQTuneCoreR4p0:: ~PQTuneCoreR4p0()
{
	delete hsv;
}
