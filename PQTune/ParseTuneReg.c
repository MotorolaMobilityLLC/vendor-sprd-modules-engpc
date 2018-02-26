#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <utils/Log.h>
#include "PQTuningParmDefine.h"

int parse_gamma_reg(struct gamma_common *gamma)
{
	int fd;
	u32 sizes;
	u8* data;
	int cnt;

	fd = open(DpuGamma, O_RDWR);

	if(fd < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	sizes = sizeof(gamma->gamma);
	cnt = read(fd, &gamma->gamma, sizes);
	ENG_LOG("parse_gamma_reg cnt0 %d sizes %d \n", cnt, sizes);
	close(fd);

	return 0;
}

int update_gamma_reg(struct gamma_common *gamma)
{
	int fd;
	u32 sizes;
	u8* data;
	int cnt;

	fd = open(DpuGamma, O_RDWR);

	if(fd < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	sizes = sizeof(gamma->gamma);
	cnt = write(fd, &gamma->gamma, sizes);
	ENG_LOG("update_gamma_reg cnt0 %d sizes %d \n", cnt, sizes);
	close(fd);

	return 0;
}

int parse_abc_reg(struct abc_common *abc)
{
	int fd;
	u32 sizes;
	u8* data;

	fd = open(DpuSlp, O_RDWR);
	if(fd < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	sizes = sizeof(abc->cfg);
	read(fd, (u8 *)&abc->cfg, sizes);
	close(fd);

	return 0;
}

int update_abc_reg(struct abc_common *abc)
{
	int fd;
	u32 sizes;
	u8* data;

	fd = open(DpuSlp, O_RDWR);
	if(fd < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	sizes = sizeof(abc->cfg);
	write(fd, (u8 *)&abc->cfg, sizes);
	close(fd);

	return 0;

}

int parse_cms_reg(struct cms_common *cms)
{
	int fd0, fd1;
	u32 sizes0,sizes1;
	u8* data;
	int cnt;

	fd0 = open(DpuHsv, O_RDWR);
	fd1 = open(DpuCm, O_RDWR);
	if(fd0 < 0 || fd1 < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	sizes0 = sizeof(cms->hsvcm[0].hsv);
	sizes1 = sizeof(cms->hsvcm[0].cm);
	ENG_LOG("parse_cms_reg sizes0 %d: size1 %d\n", sizes0, sizes1);
	cnt = read(fd0, &(cms->hsvcm[0].hsv), sizes0);
	ENG_LOG("parse_cms_reg cnt0 %d \n", cnt);
	cnt = read(fd1, &(cms->hsvcm[0].cm), sizes1);
	ENG_LOG("parse_cms_reg cnt %d\n", cnt);
	close(fd0);
	close(fd1);

	return 0;

}

int update_cms_reg(struct cms_common *cms)
{
	int fd0, fd1;
	u32 sizes0,sizes1;
	u8* data;

	fd0 = open(DpuHsv, O_RDWR);
	fd1 = open(DpuCm, O_RDWR);
	if(fd0 < 0 || fd1 < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	sizes0 = sizeof(cms->hsvcm[0].hsv);
	sizes1 = sizeof(cms->hsvcm[0].cm);
	write(fd0, (u8 *)&cms->hsvcm[0].hsv, sizes0);
	write(fd1, (u8 *)&cms->hsvcm[0].cm, sizes1);
	close(fd0);
	close(fd1);

	return 0;

}

int parse_bld_reg(struct bld_common *bld)
{
	int fd0, fd1;
	u32 sizes0,sizes1;
	u8* data;

	fd0 = open(DpuHsv, O_RDWR);
	fd1 = open(DpuCm, O_RDWR);
	if(fd0 < 0 || fd1 < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	sizes0 = sizeof(bld->hsvcm[0].hsv);
	sizes1 = sizeof(bld->hsvcm[0].cm);
	read(fd0, (u8 *)&bld->hsvcm[0].hsv, sizes0);
	read(fd1, (u8 *)&bld->hsvcm[0].cm, sizes1);
	close(fd0);
	close(fd1);

	return 0;
}

int update_bld_reg(struct bld_common *bld)
{
	int fd0, fd1;
	u32 sizes0,sizes1;
	u8* data;

	fd0 = open(DpuHsv, O_RDWR);
	fd1 = open(DpuCm, O_RDWR);

	if(fd0 < 0 || fd1 < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	sizes0 = sizeof(bld->hsvcm[0].hsv);
	sizes1 = sizeof(bld->hsvcm[0].cm);
	write(fd0, (u8 *)&bld->hsvcm[0].hsv, sizes0);
	write(fd1, (u8 *)&bld->hsvcm[0].cm, sizes1);
	close(fd0);
	close(fd1);
	return 0;
}
