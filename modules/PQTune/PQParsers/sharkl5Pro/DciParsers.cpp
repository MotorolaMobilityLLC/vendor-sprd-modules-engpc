#include <utils/Log.h>
#include "ParserCore.h"

#define PARSE_EPF_DCI_CONFIG(X, I) \
({ \
	if (xmlHasProp(propNode, BAD_CAST #X)) { \
		szPropity = xmlGetProp(propNode, (const xmlChar*) #X); \
		dci->epf[I].X = strtoul((char *)szPropity, NULL, 0); \
		xmlFree(szPropity); \
		propNode = propNode->next; \
	} \
})

#define PARSE_SLP_CONFIG(X, I) \
({ \
	if (xmlHasProp(propNode, BAD_CAST #X)) { \
		szPropity = xmlGetProp(propNode, (const xmlChar*) #X); \
		dci->slp[I].X = strtoul((char *)szPropity, NULL, 0); \
		xmlFree(szPropity); \
		propNode = propNode->next; \
	} \
})

#define PARSE_LTM_CONFIG(X, I) \
({ \
	if (xmlHasProp(propNode, BAD_CAST #X)) { \
		szPropity = xmlGetProp(propNode, (const xmlChar*) #X); \
		dci->ltm[I].X = strtoul((char *)szPropity, NULL, 0); \
		xmlFree(szPropity); \
		propNode = propNode->next; \
	} \
})

#define UPDATE_SLP_CONFIG(X, I) \
({ \
	if (xmlHasProp(propNode, BAD_CAST #X)) { \
		snprintf(numStr, sizeof(numStr), "%d", dci->slp[I].X); \
		xmlSetProp(propNode, BAD_CAST #X, (const xmlChar*)numStr); \
		propNode = propNode->next; \
	} \
})

#define UPDATE_LTM_CONFIG(X, I) \
({ \
	if (xmlHasProp(propNode, BAD_CAST #X)) { \
		snprintf(numStr, sizeof(numStr), "%d", dci->ltm[I].X); \
		xmlSetProp(propNode, BAD_CAST #X, (const xmlChar*)numStr); \
		propNode = propNode->next; \
	} \
})

#define UPDATE_EPF_CONFIG(X, I) \
({ \
	if (xmlHasProp(propNode, BAD_CAST #X)) { \
		snprintf(numStr, sizeof(numStr), "%d", dci->epf[I].X); \
		xmlSetProp(propNode, BAD_CAST #X, (const xmlChar*)numStr); \
		propNode = propNode->next; \
	} \
})

static int parse_dci_version(dci_common_sharkl5Pro *dci, xmlNodePtr curNode)
{
	xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;

	ALOGD("curNode name %s \n",curNode->name);
	subNode = curNode; //subNode table
	while(!xmlStrcmp(subNode->name, BAD_CAST"enhance")) {
		if(xmlHasProp(subNode, BAD_CAST "version")) {
			szPropity = xmlGetProp(subNode, (const xmlChar*) "version");
			dci->version.version = strtoul((char *)szPropity, NULL, 0);
			xmlFree(szPropity);
		}
		subNode = subNode->next;
	}
	return 0;
}

static int parse_hsv_regs_table(dci_common_sharkl5Pro *dci, xmlNodePtr curNode)
{
	int i = 0;
	int j = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;

	subNode = curNode->children;  //param
	while(NULL != subNode) {
		attrPtr = subNode->properties;  //r
		while (NULL != attrPtr) {
			if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"r")) {
				szPropity = xmlGetProp(subNode, (const xmlChar*)"r");
				dci->lut3d.r[j] = strtoul((char *)szPropity, NULL, 0);
				xmlFree(szPropity);
			} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"g")) {
				szPropity = xmlGetProp(subNode, (const xmlChar*)"g");
				dci->lut3d.g[j] = strtoul((char *)szPropity, NULL, 0);
				xmlFree(szPropity);
			} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"b")) {
				szPropity = xmlGetProp(subNode, (const xmlChar*)"b");
				dci->lut3d.b[j] = strtoul((char *)szPropity, NULL, 0);
				xmlFree(szPropity);
			}
			attrPtr = attrPtr->next;
		}
		j++;
		subNode = subNode->next;
	}

	return 0;
}

static int parse_epf_cfg_arrays(dci_common_sharkl5Pro *dci, xmlNodePtr curNode)
{
	xmlNodePtr propNode;
	xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;
	char numStr[10];
	int j = 0;

	subNode = curNode->children;//number index
	while(NULL != subNode) {
		propNode = subNode->children;  //param
		while (NULL != propNode){
			PARSE_EPF_DCI_CONFIG(epsilon0, j);
			PARSE_EPF_DCI_CONFIG(epsilon1, j);
			PARSE_EPF_DCI_CONFIG(gain0, j);
			PARSE_EPF_DCI_CONFIG(gain1, j);
			PARSE_EPF_DCI_CONFIG(gain2, j);
			PARSE_EPF_DCI_CONFIG(gain3, j);
			PARSE_EPF_DCI_CONFIG(gain4, j);
			PARSE_EPF_DCI_CONFIG(gain5, j);
			PARSE_EPF_DCI_CONFIG(gain6, j);
			PARSE_EPF_DCI_CONFIG(gain7, j);
			PARSE_EPF_DCI_CONFIG(max_diff, j);
			PARSE_EPF_DCI_CONFIG(min_diff, j);
		}
		subNode = subNode->next;
		j++;
	}

return 0;
}

static int parse_slp_cfg_arrays(dci_common_sharkl5Pro *dci, xmlNodePtr curNode)
{
	xmlNodePtr propNode;
	xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;
	int j = 0;

	subNode = curNode->children;
	while(NULL != subNode) {
		propNode = subNode->children;
		PARSE_SLP_CONFIG(hist_exb_no, j);
		PARSE_SLP_CONFIG(hist_exb_percent, j);
		PARSE_SLP_CONFIG(hist9_index0, j);
		PARSE_SLP_CONFIG(hist9_index1, j);
		PARSE_SLP_CONFIG(hist9_index2, j);
		PARSE_SLP_CONFIG(hist9_index3, j);
		PARSE_SLP_CONFIG(hist9_index4, j);
		PARSE_SLP_CONFIG(hist9_index5, j);
		PARSE_SLP_CONFIG(hist9_index6, j);
		PARSE_SLP_CONFIG(hist9_index7, j);
		PARSE_SLP_CONFIG(hist9_index8, j);
		PARSE_SLP_CONFIG(glb_x1, j);
		PARSE_SLP_CONFIG(glb_x2, j);
		PARSE_SLP_CONFIG(glb_x3, j);
		PARSE_SLP_CONFIG(glb_s1, j);
		PARSE_SLP_CONFIG(glb_s2, j);
		PARSE_SLP_CONFIG(glb_s3, j);
		PARSE_SLP_CONFIG(fast_ambient_th, j);
		PARSE_SLP_CONFIG(screen_change_percent_th, j);
		PARSE_SLP_CONFIG(mask_height, j);
		PARSE_SLP_CONFIG(brightness, j);
		PARSE_SLP_CONFIG(brightness_step, j);
		PARSE_SLP_CONFIG(first_max_bright_th, j);
		PARSE_SLP_CONFIG(first_max_bright_th_step0, j);
		PARSE_SLP_CONFIG(first_max_bright_th_step1, j);
		PARSE_SLP_CONFIG(first_max_bright_th_step2, j);
		PARSE_SLP_CONFIG(first_max_bright_th_step3, j);
		PARSE_SLP_CONFIG(first_max_bright_th_step4, j);
		PARSE_SLP_CONFIG(local_weight, j);
		PARSE_SLP_CONFIG(first_percent_th, j);
		PARSE_SLP_CONFIG(first_pth_index0, j);
		PARSE_SLP_CONFIG(first_pth_index1, j);
		PARSE_SLP_CONFIG(first_pth_index2, j);
		PARSE_SLP_CONFIG(first_pth_index3, j);

		subNode = subNode->next;
		j++;
	}

	ENG_LOG("l5pro jkjkjkjk %d %s\n", j, __func__);
	return 0;
}

static int parse_ltm_cfg_arrays(dci_common_sharkl5Pro *dci, xmlNodePtr curNode)
{
	xmlNodePtr propNode;
	xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;
	int j = 0;

	subNode = curNode->children;//number
	while(subNode) {
		propNode = subNode->children;//param
		while (NULL != propNode){
			PARSE_LTM_CONFIG(slp_step_clip, j);
			PARSE_LTM_CONFIG(slp_high_clip, j);
			PARSE_LTM_CONFIG(slp_low_clip, j);
			if (propNode)
				propNode = propNode->next;
		}
		subNode = subNode->next;
		j++;
	}

	return 0;

}

static int update_dci_version(dci_common_sharkl5Pro *dci, xmlNodePtr curNode)
{
    int i = 0;
    xmlNodePtr subNode;
    char numStr[12];

    ALOGD("ggaammaa version = %d \n", dci->version.version);
    subNode = curNode; //subNode table
	while(!xmlStrcmp(subNode->name, BAD_CAST"enhance")) {
		if(xmlHasProp(subNode, BAD_CAST "version")) {
            snprintf(numStr, sizeof(numStr), "%d", dci->version.version);
            xmlSetProp(subNode, BAD_CAST "version", (const xmlChar*)numStr);
        }
		subNode = subNode->next;
	}
        return 0;
}

static int update_hsv_regs_table(dci_common_sharkl5Pro *dci, xmlNodePtr curNode)
{
	int j = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];

	subNode = curNode->children; //param
	while(NULL != subNode) {
        attrPtr = subNode->properties;   //r
        while (NULL != attrPtr) {
            if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"r")) {
                snprintf(numStr, sizeof(numStr), "%d", dci->lut3d.r[j]);
                xmlSetProp(subNode, BAD_CAST "r", (const xmlChar*)numStr);
            } else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"g")) {
                snprintf(numStr, sizeof(numStr), "%d", dci->lut3d.g[j]);
                xmlSetProp(subNode, BAD_CAST "g", (const xmlChar*)numStr);
            } else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"b")) {
                snprintf(numStr, sizeof(numStr), "%d", dci->lut3d.b[j]);
                xmlSetProp(subNode, BAD_CAST "b", (const xmlChar*)numStr);
            }
            attrPtr = attrPtr->next;
        }
        j++;
        subNode = subNode->next;
    }

	return 0;
}

static int update_slp_cfg_arrays(dci_common_sharkl5Pro *dci, xmlNodePtr curNode)
{
	xmlNodePtr propNode;
	xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;
	char numStr[10];
	int j = 0;

	subNode = curNode->children;//number index
	while(NULL != subNode) {
		propNode = subNode->children;
		while (NULL != propNode){
			UPDATE_SLP_CONFIG(hist_exb_no, j);
			UPDATE_SLP_CONFIG(hist_exb_percent, j);
			UPDATE_SLP_CONFIG(hist9_index0, j);
			UPDATE_SLP_CONFIG(hist9_index1, j);
			UPDATE_SLP_CONFIG(hist9_index2, j);
			UPDATE_SLP_CONFIG(hist9_index3, j);
			UPDATE_SLP_CONFIG(hist9_index4, j);
			UPDATE_SLP_CONFIG(hist9_index5, j);
			UPDATE_SLP_CONFIG(hist9_index6, j);
			UPDATE_SLP_CONFIG(hist9_index7, j);
			UPDATE_SLP_CONFIG(hist9_index8, j);
			UPDATE_SLP_CONFIG(glb_x1, j);
			UPDATE_SLP_CONFIG(glb_x2, j);
			UPDATE_SLP_CONFIG(glb_x3, j);
			UPDATE_SLP_CONFIG(glb_s1, j);
			UPDATE_SLP_CONFIG(glb_s2, j);
			UPDATE_SLP_CONFIG(glb_s3, j);
			UPDATE_SLP_CONFIG(fast_ambient_th, j);
			UPDATE_SLP_CONFIG(screen_change_percent_th, j);
			UPDATE_SLP_CONFIG(mask_height, j);
			UPDATE_SLP_CONFIG(brightness, j);
			UPDATE_SLP_CONFIG(brightness_step, j);
			UPDATE_SLP_CONFIG(first_max_bright_th, j);
			UPDATE_SLP_CONFIG(first_max_bright_th_step0, j);
			UPDATE_SLP_CONFIG(first_max_bright_th_step1, j);
			UPDATE_SLP_CONFIG(first_max_bright_th_step2, j);
			UPDATE_SLP_CONFIG(first_max_bright_th_step3, j);
			UPDATE_SLP_CONFIG(first_max_bright_th_step4, j);
			UPDATE_SLP_CONFIG(local_weight, j);
			UPDATE_SLP_CONFIG(first_percent_th, j);
			UPDATE_SLP_CONFIG(first_pth_index0, j);
			UPDATE_SLP_CONFIG(first_pth_index1, j);
			UPDATE_SLP_CONFIG(first_pth_index2, j);
			UPDATE_SLP_CONFIG(first_pth_index3, j);

		}
		subNode = subNode->next;
		j++;
	}
	return 0;
}

static int update_ltm_cfg_arrays(dci_common_sharkl5Pro *dci, xmlNodePtr curNode)
{
	xmlNodePtr subNode;
	xmlNodePtr propNode = NULL;
	xmlNodePtr NextpropNode;
	xmlNodePtr childNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];
	int j = 0;

	subNode = curNode->children;
	while(NULL != subNode) {
		propNode = subNode->children;
		while (NULL != propNode){
			UPDATE_LTM_CONFIG(slp_step_clip, j);
			UPDATE_LTM_CONFIG(slp_high_clip, j);
			UPDATE_LTM_CONFIG(slp_low_clip, j);
		}
		subNode = subNode->next;
		j++;
	}

	return 0;
}

static int update_epf_cfg_arrays(dci_common_sharkl5Pro *dci, xmlNodePtr curNode)
{
	xmlNodePtr subNode;
	xmlNodePtr propNode = NULL;
	xmlNodePtr NextpropNode;
	xmlNodePtr childNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];
	int j = 0;

	subNode = curNode->children;
	while(NULL != subNode) {
		propNode = subNode->children;
		while (NULL != propNode){
			UPDATE_EPF_CONFIG(epsilon0, j);
			UPDATE_EPF_CONFIG(epsilon1, j);
			UPDATE_EPF_CONFIG(gain0, j);
			UPDATE_EPF_CONFIG(gain1, j);
			UPDATE_EPF_CONFIG(gain2, j);
			UPDATE_EPF_CONFIG(gain3, j);
			UPDATE_EPF_CONFIG(gain4, j);
			UPDATE_EPF_CONFIG(gain5, j);
			UPDATE_EPF_CONFIG(gain6, j);
			UPDATE_EPF_CONFIG(gain7, j);
			UPDATE_EPF_CONFIG(max_diff, j);
			UPDATE_EPF_CONFIG(min_diff, j);
		}
		subNode = subNode->next;
		j++;
	}
	return 0;
}

int DciParser::parse_reg(uint08_t *ctx)
{
	int fdslp, fdltm, fdepf, fdLut3d;
	unsigned int szslp, szltm, szepf, slut3d;
	unsigned char* data;
	int cnt;
	unsigned short tmp,tmp1;
	dci_common_sharkl5Pro *dci;

	dci = &((pq_tuning_parm_sharkl5Pro *)ctx)->dci;

	fdslp = open(DpuSlp, O_RDWR);
	fdltm = open(DpuLtm, O_RDWR);
	fdepf = open(DpuEpf, O_RDWR);
	fdLut3d = open(DpuLut3d, O_RDWR);

	if (fdslp < 0 || fdepf < 0 || fdltm < 0 || fdLut3d < 0) {
		if (fdslp >= 0)
			close(fdslp);
		if (fdltm >= 0)
			close(fdltm);
		if (fdepf >= 0)
			close(fdepf);
		if (fdLut3d >= 0)
			close(fdLut3d);
		ALOGD("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}

	szslp = sizeof(slp_params_l5pro);
	szltm = sizeof(ltm_params_l5pro);
	szepf = sizeof(dci->epf);
	slut3d = sizeof(lut3d_l5pro);

	memset(&slp_params_l5pro, 0, szslp);
	memset(&ltm_params_l5pro, 0, szltm);
	memset(&lut3d_l5pro, 0, slut3d);

	cnt = read(fdepf, &(dci->epf), szepf);
	cnt = read(fdslp, (unsigned char*)&slp_params_l5pro, szslp);
	if (cnt != szslp)
		ALOGD("L5Pro read slp fail regs_size %d read_cnt %d\n", szslp, cnt);
	cnt = read(fdltm, (unsigned char*)&ltm_params_l5pro, szltm);
	if (cnt != szltm)
		ALOGD("L5Pro read ltm fail regs_size %d read_cnt %d\n", szltm, cnt);
	cnt = read(fdLut3d, &lut3d_l5pro, slut3d);

	dci->slp[0].hist_exb_no = slp_params_l5pro.hist_exb_no;
	dci->slp[0].hist_exb_percent = slp_params_l5pro.hist_exb_percent;
	dci->slp[0].hist9_index0 = slp_params_l5pro.hist9_index[0];
	dci->slp[0].hist9_index1 = slp_params_l5pro.hist9_index[1];
	dci->slp[0].hist9_index2 = slp_params_l5pro.hist9_index[2];
	dci->slp[0].hist9_index3 = slp_params_l5pro.hist9_index[3];
	dci->slp[0].hist9_index4 = slp_params_l5pro.hist9_index[4];
	dci->slp[0].hist9_index5 = slp_params_l5pro.hist9_index[5];
	dci->slp[0].hist9_index6 = slp_params_l5pro.hist9_index[6];
	dci->slp[0].hist9_index7 = slp_params_l5pro.hist9_index[7];
	dci->slp[0].hist9_index8 = slp_params_l5pro.hist9_index[8];
	dci->slp[0].glb_s1 = slp_params_l5pro.glb_s[0];
	dci->slp[0].glb_s2 = slp_params_l5pro.glb_s[1];
	dci->slp[0].glb_s3 = slp_params_l5pro.glb_s[2];
	dci->slp[0].glb_x1 = slp_params_l5pro.glb_x[0];
	dci->slp[0].glb_x2 = slp_params_l5pro.glb_x[1];
	dci->slp[0].glb_x3 = slp_params_l5pro.glb_x[2];
	dci->slp[0].fast_ambient_th = slp_params_l5pro.fast_ambient_th;
	dci->slp[0].screen_change_percent_th = slp_params_l5pro.scene_change_percent_th;
	dci->slp[0].mask_height = slp_params_l5pro.mask_height;
	dci->slp[0].brightness = slp_params_l5pro.brightness;
	dci->slp[0].brightness_step = slp_params_l5pro.brightness_step;
	dci->slp[0].first_max_bright_th = slp_params_l5pro.fst_max_bright_th;
	dci->slp[0].first_max_bright_th_step0 = slp_params_l5pro.fst_max_bright_th_step[0];
	dci->slp[0].first_max_bright_th_step1 = slp_params_l5pro.fst_max_bright_th_step[1];
	dci->slp[0].first_max_bright_th_step2 = slp_params_l5pro.fst_max_bright_th_step[2];
	dci->slp[0].first_max_bright_th_step3 = slp_params_l5pro.fst_max_bright_th_step[3];
	dci->slp[0].first_max_bright_th_step4 = slp_params_l5pro.fst_max_bright_th_step[4];
	dci->slp[0].local_weight = slp_params_l5pro.local_weight;
	dci->slp[0].first_percent_th = slp_params_l5pro.fst_pth;
	dci->slp[0].first_pth_index0 = slp_params_l5pro.fst_pth_index[0];
	dci->slp[0].first_pth_index1 = slp_params_l5pro.fst_pth_index[1];
	dci->slp[0].first_pth_index2 = slp_params_l5pro.fst_pth_index[2];
	dci->slp[0].first_pth_index3 = slp_params_l5pro.fst_pth_index[3];

	dci->ltm[0].slp_step_clip = ltm_params_l5pro.limit_clip_step;
	dci->ltm[0].slp_high_clip = ltm_params_l5pro.limit_hclip;
	dci->ltm[0].slp_low_clip = ltm_params_l5pro.limit_lclip;

	for (cnt = 0; cnt < 729; cnt++) {
		dci->lut3d.r[cnt] = ((lut3d_l5pro.value[cnt] >> 20)& 0x3ff);
		dci->lut3d.g[cnt] = ((lut3d_l5pro.value[cnt] >> 10) & 0x3ff);
		dci->lut3d.b[cnt] = (lut3d_l5pro.value[cnt] & 0x3ff);
	}

	close(fdLut3d);
	close(fdslp);
	close(fdltm);
	close(fdepf);

	return 0;

}

int DciParser::update_reg(uint08_t *ctx)
{
	int fdLut3d, fdcm, fdslp, fdltm, fdepf;
	unsigned int szlut3d, szcm, szslp, szltm, szepf;
	unsigned char* data;
	unsigned int disable;
	unsigned short tmp,tmp1;
	int cnt;

	dci_common_sharkl5Pro *dci;

	dci = &((pq_tuning_parm_sharkl5Pro *)ctx)->dci;

	if (dci->version.enable) {
		fdLut3d = open(DpuLut3d, O_RDWR);
		fdslp = open(DpuSlp, O_RDWR);
		fdltm = open(DpuLtm, O_RDWR);
		fdepf = open(DpuEpf, O_RDWR);

		if (fdLut3d < 0 || fdslp < 0 || fdepf < 0 || fdltm < 0) {
			if (fdLut3d >= 0)
				close(fdLut3d);
			if (fdslp >= 0)
				close(fdslp);
			if (fdltm >= 0)
				close(fdltm);
			if (fdepf >= 0)
				close(fdepf);
			ALOGD("%s: open file failed, err: %s\n", __func__, strerror(errno));
			return errno;
		}
		szlut3d = sizeof(lut3d_l5pro);
		szslp = sizeof(slp_params_l5pro);
		szslp = szslp - 2;
		szltm = sizeof(ltm_params_l5pro);
		szepf = sizeof(dci->epf);

		memset(&lut3d_l5pro, 0, sizeof(lut3d_l5pro));
		for (cnt = 0; cnt < 729; cnt++) {
			lut3d_l5pro.value[cnt] |=  ((dci->lut3d.r[cnt] & 0x3ff) << 20);
			lut3d_l5pro.value[cnt] |=  (dci->lut3d.g[cnt] & 0x3ff) << 10;
			lut3d_l5pro.value[cnt] |=  dci->lut3d.b[cnt] & 0x3ff;
		}

		write(fdLut3d, (unsigned char *)&lut3d_l5pro, szlut3d);
		write(fdepf, (unsigned char *)&dci->epf, szepf);

		slp_params_l5pro.hist_exb_no = dci->slp[0].hist_exb_no;
		slp_params_l5pro.hist_exb_percent = dci->slp[0].hist_exb_percent;
		slp_params_l5pro.hist9_index[0] = dci->slp[0].hist9_index0;
		slp_params_l5pro.hist9_index[1] = dci->slp[0].hist9_index1;
		slp_params_l5pro.hist9_index[2] = dci->slp[0].hist9_index2;
		slp_params_l5pro.hist9_index[3] = dci->slp[0].hist9_index3;
		slp_params_l5pro.hist9_index[4] = dci->slp[0].hist9_index4;
		slp_params_l5pro.hist9_index[5] = dci->slp[0].hist9_index5;
		slp_params_l5pro.hist9_index[6] = dci->slp[0].hist9_index6;
		slp_params_l5pro.hist9_index[7] = dci->slp[0].hist9_index7;
		slp_params_l5pro.hist9_index[8] = dci->slp[0].hist9_index8;
		slp_params_l5pro.glb_s[0] = dci->slp[0].glb_s1;
		slp_params_l5pro.glb_s[1] = dci->slp[0].glb_s2;
		slp_params_l5pro.glb_s[2] = dci->slp[0].glb_s3;
		slp_params_l5pro.glb_x[0] = dci->slp[0].glb_x1;
		slp_params_l5pro.glb_x[1] = dci->slp[0].glb_x2;
		slp_params_l5pro.glb_x[2] = dci->slp[0].glb_x3;
		slp_params_l5pro.fast_ambient_th = dci->slp[0].fast_ambient_th;
		slp_params_l5pro.scene_change_percent_th = dci->slp[0].screen_change_percent_th;
		slp_params_l5pro.mask_height = dci->slp[0].mask_height;
		slp_params_l5pro.brightness = dci->slp[0].brightness;
		slp_params_l5pro.brightness_step = dci->slp[0].brightness_step;
		slp_params_l5pro.fst_max_bright_th = dci->slp[0].first_max_bright_th;
		slp_params_l5pro.fst_max_bright_th_step[0] = dci->slp[0].first_max_bright_th_step0;
		slp_params_l5pro.fst_max_bright_th_step[1] = dci->slp[0].first_max_bright_th_step1;
		slp_params_l5pro.fst_max_bright_th_step[2] = dci->slp[0].first_max_bright_th_step2;
		slp_params_l5pro.fst_max_bright_th_step[3] = dci->slp[0].first_max_bright_th_step3;
		slp_params_l5pro.fst_max_bright_th_step[4] = dci->slp[0].first_max_bright_th_step4;
		slp_params_l5pro.local_weight = dci->slp[0].local_weight;
		slp_params_l5pro.fst_pth = dci->slp[0].first_percent_th;
		slp_params_l5pro.fst_pth_index[0] = dci->slp[0].first_pth_index0;
		slp_params_l5pro.fst_pth_index[1] = dci->slp[0].first_pth_index1;
		slp_params_l5pro.fst_pth_index[2] = dci->slp[0].first_pth_index2;
		slp_params_l5pro.fst_pth_index[3] = dci->slp[0].first_pth_index3;

		cnt = write(fdslp, (unsigned char*)&slp_params_l5pro, szslp);
		if (cnt != szslp)
			ALOGD("write slp fail regs_size %d wr_cnt %d\n", szslp, cnt);

		ltm_params_l5pro.limit_clip_step = dci->ltm[0].slp_step_clip;
		ltm_params_l5pro.limit_hclip = dci->ltm[0].slp_high_clip;
		ltm_params_l5pro.limit_lclip = dci->ltm[0].slp_low_clip;
		cnt = write(fdltm, (unsigned char*)&ltm_params_l5pro, szltm);
		if (cnt != szltm)
			ALOGD("write ltm fail regs_size %d wr_cnt %d\n", szltm, cnt);

		for (cnt = 0; cnt < 729; cnt++) {
			lut3d_l5pro.value[cnt] |=  ((dci->lut3d.r[cnt] & 0x3ff) << 20);
			lut3d_l5pro.value[cnt] |=  (dci->lut3d.g[cnt] & 0x3ff) << 10;
			lut3d_l5pro.value[cnt] |=  dci->lut3d.b[cnt] & 0x3ff;
		}
		cnt = write(fdLut3d, &lut3d_l5pro, szlut3d);

		close(fdLut3d);
		close(fdslp);
		close(fdltm);
		close(fdepf);
	} else {
		fdslp = open(PQDisable, O_WRONLY);
		if(fdslp < 0) {
			ALOGD("%s: fd2 open file failed, err: %s\n", __func__, strerror(errno));
			return errno;
		}
		disable = SLP_EN | EPF_EN | LTM_EN | LUT3D_EN;
		write(fdslp, &disable, sizeof(disable));
		close(fdslp);
	}

	return 0;
}

int DciParser::update_xml(uint08_t *ctx)
{
	xmlDocPtr doc;
	xmlNodePtr curNode;
	xmlNodePtr tmpNode;
	dci_common_sharkl5Pro *dci;

	dci = &((pq_tuning_parm_sharkl5Pro *)ctx)->dci;

	doc = xmlReadFile(dci_xml, "utf-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
		ALOGD("Document not parsed successfully.\n");
		return -1;
	}

	curNode = xmlDocGetRootElement(doc);

	if (xmlStrcmp(curNode->name, (const xmlChar*)"root"))
	{
		ALOGD("dci_config node != root\n");
		xmlFreeDoc(doc);
		return -1;
	}

	curNode = curNode->children;

	if(tmpNode = FindNode(curNode, "epf"))
		update_epf_cfg_arrays(dci, tmpNode);
	if(tmpNode = FindNode(curNode, "slp"))
		update_slp_cfg_arrays(dci, tmpNode);
	if(tmpNode = FindNode(curNode, "ltm"))
		update_ltm_cfg_arrays(dci, tmpNode);
	if(tmpNode = FindNode(curNode, "hsv_table"))
		update_hsv_regs_table(dci, tmpNode);
	if(tmpNode = FindNode(curNode, "enhance"))
		update_dci_version(dci, tmpNode);

	xmlSaveFormatFileEnc(dci_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	return 0;
}

int DciParser::parse_xml(uint08_t *ctx)
{
	xmlDocPtr doc;
	xmlNodePtr curNode;
	xmlNodePtr tmpNode;
	dci_common_sharkl5Pro *dci;

	dci = &((pq_tuning_parm_sharkl5Pro *)ctx)->dci;

	doc = xmlReadFile(dci_xml, "utf-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
		ALOGD("Document not parsed successfully.\n");
		return -1;
	}

	curNode = xmlDocGetRootElement(doc);

	if (xmlStrcmp(curNode->name, (const xmlChar*)"root")) {
		ALOGD("dci_config node != root\n");
		xmlFreeDoc(doc);
		return -1;
	}

	curNode = curNode->children;
	if(tmpNode = FindNode(curNode, "epf"))
		parse_epf_cfg_arrays(dci, tmpNode);
	if(tmpNode = FindNode(curNode, "slp"))
		parse_slp_cfg_arrays(dci, tmpNode);
	if(tmpNode = FindNode(curNode, "ltm"))
		parse_ltm_cfg_arrays(dci, tmpNode);
	if(tmpNode = FindNode(curNode, "hsv_table"))
		parse_hsv_regs_table(dci, tmpNode);
	if(tmpNode = FindNode(curNode, "enhance"))
		parse_dci_version(dci, tmpNode);

	xmlSaveFormatFileEnc(dci_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	ALOGD("this is pq\n");
	return 0;
}
