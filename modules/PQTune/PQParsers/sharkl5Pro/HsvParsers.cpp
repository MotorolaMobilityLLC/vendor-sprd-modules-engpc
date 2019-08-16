#include <utils/Log.h>
#include "ParserCore.h"

static int parse_hsv_version(hsv_common_sharkl5Pro *hsv, xmlNodePtr curNode)
{
	xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;

	ALOGD("curNode name %s \n",curNode->name);
	subNode = curNode; //subNode table
	while(!xmlStrcmp(subNode->name, BAD_CAST"enhance")) {
		if(xmlHasProp(subNode, BAD_CAST "version")) {
			szPropity = xmlGetProp(subNode, (const xmlChar*) "version");
			hsv->version.version = strtoul((char *)szPropity, NULL, 0);
			ALOGD("aaaaaa hsv verison %d \n", hsv->version.version);
			free(szPropity);
		}
		subNode = subNode->next;
	}
	return 0;
}

static int parse_hsv_regs_table(hsv_common_sharkl5Pro *hsv, xmlNodePtr curNode)
{
	int i = 0;
	int j = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;

	ALOGD("curNode name %s \n",curNode->name);
	subNode = curNode; //subNode table
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ALOGD("curNode name %s \n",subNode->name);
			propNode = subNode->children;
			while (NULL != propNode) {
				attrPtr = propNode->properties;
				while (NULL != attrPtr) {
                	if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"r")) {
                    	szPropity = xmlGetProp(propNode, (const xmlChar*)"r");
						hsv->lut3d[i].r[j] = strtoul((char *)szPropity, NULL, 0);
						free(szPropity);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"g")) {
						szPropity = xmlGetProp(propNode, (const xmlChar*)"g");
						hsv->lut3d[i].g[j] = strtoul((char *)szPropity, NULL, 0);
						free(szPropity);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"b")) {
						szPropity = xmlGetProp(propNode, (const xmlChar*)"b");
						hsv->lut3d[i].b[j] = strtoul((char *)szPropity, NULL, 0);
						free(szPropity);
					}
					attrPtr = attrPtr->next;
            	}
				i++;
				propNode = propNode->next;
			}
		}
		j = 0;
		i++;
		subNode = subNode->next;
	}

	return 0;
}

static int update_hsv_version(hsv_common_sharkl5Pro *hsv, xmlNodePtr curNode)
{
    int i = 0;
    xmlNodePtr subNode;
    char numStr[12];

    ALOGD("ggaammaa version = %d \n", hsv->version.version);
    subNode = curNode; //subNode table
	while(!xmlStrcmp(subNode->name, BAD_CAST"enhance")) {
		if(xmlHasProp(subNode, BAD_CAST "version")) {
            snprintf(numStr, sizeof(numStr), "%d", hsv->version.version);
            xmlSetProp(subNode, BAD_CAST "version", (const xmlChar*)numStr);
        }
		subNode = subNode->next;
	}
        return 0;
}

static int update_hsv_regs_table(hsv_common_sharkl5Pro *hsv, xmlNodePtr curNode)
{
	int i = 0;
	int j = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];

	ALOGD("curNode name %s \n",curNode->name);
	subNode = curNode; //subNode table
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ALOGD("curNode name %s \n",subNode->name);
			propNode = subNode->children;
			while (NULL != propNode) {
				attrPtr = propNode->properties;
				while (NULL != attrPtr) {
                	if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"r")) {
						snprintf(numStr, sizeof(numStr), "%d", hsv->lut3d[i].r[j]);
						xmlSetProp(propNode, BAD_CAST "r", (const xmlChar*)numStr);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"g")) {
						snprintf(numStr, sizeof(numStr), "%d", hsv->lut3d[i].g[j]);
						xmlSetProp(propNode, BAD_CAST "g", (const xmlChar*)numStr);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"b")) {
						snprintf(numStr, sizeof(numStr), "%d", hsv->lut3d[i].b[j]);
						xmlSetProp(propNode, BAD_CAST "b", (const xmlChar*)numStr);
					}
					attrPtr = attrPtr->next;
            	}
				j++;
				propNode = propNode->next;
			}
		}
		j = 0;
		i++;
		subNode = subNode->next;
	}

	return 0;
}

int HsvParser::parse_reg(uint08_t *ctx)
{
	int fd;
	uint32_t sizes;
	uint08_t* data;
	int cnt;
	hsv_common_sharkl5Pro *hsv;

	hsv = &((pq_tuning_parm_sharkl5Pro *)ctx)->hsv;

	fd = open(DpuLut3d, O_RDWR);

	if(fd < 0) {
		ALOGD("%s: open file failed, err: %s\n", __func__, strerror(errno));
		return errno;
	}
	sizes = sizeof(hsv->lut3d);
	cnt = read(fd, &hsv->lut3d, sizes);
	ALOGD("parse_hsv_reg cnt0 %d sizes %d \n", cnt, sizes);
	close(fd);

	return 0;
}

int HsvParser::update_reg(uint08_t *ctx)
{
	int fd;
	int fd1;
	uint32_t sizes;
	uint08_t* data;
	int cnt;
	uint32_t disable;
	hsv_common_sharkl5Pro *hsv;

	hsv = &((pq_tuning_parm_sharkl5Pro *)ctx)->hsv;

	if(hsv->version.enable) {
		fd = open(DpuLut3d, O_RDWR);
		if(fd < 0) {
			ALOGD("%s: open file failed, err: %s\n", __func__, strerror(errno));
			return errno;
		}
		sizes = sizeof(hsv->lut3d);
		cnt = write(fd, &hsv->lut3d, sizes);
		close(fd);
	}
	else {
		fd1 = open(PQDisable, O_WRONLY);
		if(fd1 < 0) {
			ALOGD("%s: open file fd1 failed, err: %s\n", __func__, strerror(errno));
			return errno;
		}
		disable = GAMMA_EN;
		write(fd1, &disable, sizeof(disable));
		close(fd1);
	}
	ALOGD("PQ update_hsv_reg cnt0 %d sizes %d enable =  %x\n", cnt, sizes, hsv->version.enable);

	return 0;
}


int HsvParser::update_xml(uint08_t *ctx)
{

	xmlDocPtr doc;
	xmlNodePtr curNode;
	xmlNodePtr tmpNode;
	hsv_common_sharkl5Pro *hsv;

	hsv = &((pq_tuning_parm_sharkl5Pro *)ctx)->hsv;

	doc = xmlReadFile(hsv_xml, "utf-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
		ALOGD("Document not parsed successfully.\n");
		return -1;
	}

	curNode = xmlDocGetRootElement(doc);

	if (xmlStrcmp(curNode->name, (const xmlChar*)"hsv_config"))
	{
		ALOGD("hsv_config node != root\n");
		xmlFreeDoc(doc);
		return -1;
	}

	curNode = curNode->children;

	if(tmpNode = FindNode(curNode, "reg_table"))
		update_hsv_regs_table(hsv, tmpNode);
	if(tmpNode = FindNode(curNode, "enhance"))
		update_hsv_version(hsv, tmpNode);

	xmlSaveFormatFileEnc(hsv_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	return 0;
}

int HsvParser::parse_xml(uint08_t *ctx)
{

	xmlDocPtr doc;
	xmlNodePtr curNode;
	xmlNodePtr tmpNode;
	hsv_common_sharkl5Pro *hsv;

	hsv = &((pq_tuning_parm_sharkl5Pro *)ctx)->hsv;

	doc = xmlReadFile(hsv_xml, "utf-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
		ALOGD("Document not parsed successfully.\n");
		return -1;
	}

	curNode = xmlDocGetRootElement(doc);

	if (xmlStrcmp(curNode->name, (const xmlChar*)"hsv_config")) {
		ALOGD("hsv_config node != root\n");
		xmlFreeDoc(doc);
		return -1;
	}

	curNode = curNode->children;

	if(tmpNode = FindNode(curNode, "reg_table_mode"))
		parse_hsv_regs_table(hsv, tmpNode);
	if(tmpNode = FindNode(curNode, "enhance"))
		parse_hsv_version(hsv, tmpNode);

	xmlSaveFormatFileEnc(hsv_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	ALOGD("this is pq\n");
	return 0;
}
