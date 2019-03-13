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
#include "tune_r2p0.h"

int parse_r2p0_gamma_reg(struct gamma_common *gamma)
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

int update_r2p0_gamma_reg(struct gamma_common *gamma)
{
	int fd;
	int fd1;
	u32 sizes;
	u8* data;
	int cnt;
	u32 disable;

	if(gamma->version.enable) {
		fd = open(DpuGamma, O_RDWR);
		if(fd < 0) {
			ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
			return errno;
		}
		sizes = sizeof(gamma->gamma);
		cnt = write(fd, &gamma->gamma, sizes);
		close(fd);
	}
	else {
		fd1 = open(PQDisable, O_WRONLY);
		if(fd1 < 0) {
			ENG_LOG("%s: open file fd1 failed, err: %s\n", __func__, strerror(errno));
			return errno;
		}
		disable = GAMMA_EN;
		write(fd1, &disable, sizeof(disable));
		close(fd1);
	}
	ENG_LOG("PQ update_gamma_reg cnt0 %d sizes %d enable =  %x\n", cnt, sizes, gamma->version.enable);

	return 0;
}

int parse_r2p0_abc_reg(struct abc_common *abc)
{
	int fd;
	u32 cnt;
	u32 sizes;
	u8* data;

	fd = open(DpuSlp, O_RDWR);
	if(fd < 0) {
		ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	sizes = sizeof(abc->sceneTable[0].sceneTableItem[0].slpCfg);
	cnt = read(fd, (u8 *)&abc->sceneTable[0].sceneTableItem[0].slpCfg, sizes);
	close(fd);

	if (cnt != sizes)
		return -1;
	else
		return 0;
}

int update_r2p0_abc_reg(struct abc_common *abc)
{
	int fd;
	int fd1;
	char backlight[11] = {0};
	u32 sizes;
	u8* data;
	u32 disable;

	if(abc->version.enable) {
		if (abc->slpblMode == 2) {
			fd = open(Brightness, O_RDWR);
			if (fd < 0)
				return errno;
			snprintf(backlight, sizeof(backlight), "%d", 255);
			write(fd, backlight, sizeof(backlight));
			ENG_LOG("PQ backlights  %s", backlight);
			close(fd);
		}
		 fd = open(DpuSlp, O_RDWR);
		 if(fd < 0) {
			 ENG_LOG("%s: open file failed, err: %s\n", __func__, strerror(errno));
			 return errno;
		}
		sizes = sizeof(abc->sceneTable[0].sceneTableItem[0].slpCfg);
		write(fd, (u8 *)&abc->sceneTable[0].sceneTableItem[0].slpCfg, sizes);
		close(fd);
	}
	else {
		fd1 = open(PQDisable, O_WRONLY);
		if(fd1 < 0) {
			ENG_LOG("PQ %s: open fd 1file failed, err: %s\n", __func__, strerror(errno));
			return errno;
		}
		disable = SLP_EN;
		write(fd1, &disable, sizeof(disable));
		close(fd1);
	}

	return 0;
}

int parse_r2p0_cms_reg(struct cms_common *cms)
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

int update_r2p0_cms_reg(struct cms_common *cms)
{
	int fd0, fd1, fd2;
	u32 sizes0,sizes1;
	u8* data;
	u32 disable;

	if(cms->version.enable) {
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
	} else {
		fd2 = open(PQDisable, O_WRONLY);
		if(fd2 < 0) {
			ENG_LOG("%s: fd2 open file failed, err: %s\n", __func__, strerror(errno));
			return errno;
		}
		disable = CMS_EN | HSV_EN;
		write(fd2, &disable, sizeof(disable));
		close(fd2);
	}

	return 0;
}

int parse_r2p0_bld_reg(struct bld_common *bld)
{
	int fd0, fd1;
	u32 cnt0, cnt1;
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
	cnt0 = read(fd0, (u8 *)&bld->hsvcm[0].hsv, sizes0);
	cnt1 = read(fd1, (u8 *)&bld->hsvcm[0].cm, sizes1);
	close(fd0);
	close(fd1);

	if ((cnt0 == sizes0)&&(cnt1 == sizes1))
		return 0;
	else
		return -1;
}

int update_r2p0_bld_reg(struct bld_common *bld)
{
	int fd0, fd1, fd2;
	u32 sizes0,sizes1;
	u8* data;
	u32 disable;

	ENG_LOG("PQ BLD  %s enalbe %d\n", __func__, bld->version.enable);
	if(bld->version.enable) {

		ENG_LOG("PQ BLD  %s\n", __func__);
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
	} else {
		fd2 = open(PQDisable, O_WRONLY);
		if(fd2 < 0) {
			ENG_LOG("%s: fd2 open file failed, err: %s\n", __func__, strerror(errno));
			return errno;
		}
		disable = HSV_EN | CMS_EN;
		write(fd2, &disable, sizeof(disable));
		close(fd2);
	}
	return 0;
}
