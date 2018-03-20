#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <../libxml.h>
#include <utils/Log.h>
#include "PQTuningParmDefine.h"


#define PARSE_SLP_CONFIG(X) \
({ \
	if (xmlHasProp(propNode, BAD_CAST #X)) { \
		szPropity = xmlGetProp(propNode, (const xmlChar*) #X); \
		abc->cfg.X = strtoul((char *)szPropity, (char **)&endptr, 0); \
		ENG_LOG(""#X" %d \n", abc->cfg.X); \
	} \
})

#define UPDATE_SLP_CONFIG(X)	\
({	\
	if (xmlHasProp(propNode, BAD_CAST #X)) { \
		snprintf(numStr, sizeof(numStr), "%d", abc->cfg.X); \
		xmlSetProp(propNode, BAD_CAST #X, (const xmlChar*)numStr); \
		ENG_LOG(""#X" ggggg %d \n", abc->cfg.X); \
	} \
})

static int parse_slp_config(struct abc_common *abc, xmlNodePtr curNode)
{
	xmlNodePtr propNode;
	xmlChar* szPropity;
	const char *endptr = NULL;

	propNode = curNode->children;
	while (NULL != propNode){
		PARSE_SLP_CONFIG(brightness);
		PARSE_SLP_CONFIG(conversion_matrix);
		PARSE_SLP_CONFIG(brightness_step);
		PARSE_SLP_CONFIG(second_bright_factor);
		PARSE_SLP_CONFIG(first_percent_th);
		PARSE_SLP_CONFIG(first_max_bright_th);
		propNode = propNode->next;
	}

	return 0;

}

static int parse_slp_mapping_table(struct abc_common *abc, xmlNodePtr curNode)
{
	int i = 0;
	int j = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode->children; //subNode table
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			propNode = subNode->children;
			while (NULL != propNode) {
				attrPtr = propNode->properties;
				while (NULL != attrPtr) {
                	if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"ambient")) {
                    	szPropity = xmlGetProp(propNode, (const xmlChar*)"ambient");
						abc->slp_table[i].item[j].ambient = strtoul((char *)szPropity, (char **)&endptr, 0);
						ENG_LOG("ambient %d \n", abc->slp_table[i].item[j].ambient);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"brightness")) {
						szPropity = xmlGetProp(propNode, (const xmlChar*)"brightness");
						abc->slp_table[i].item[j].slp_brightness_factor = strtoul((char *)szPropity, (char **)&endptr, 0);
						ENG_LOG("brightness %d \n", abc->slp_table[i].item[j].slp_brightness_factor);
					}
					attrPtr = attrPtr->next;
            	}
				j++;
				propNode = propNode->next;
			}
		}
		i++;
		subNode = subNode->next;
	}

	return 0;
}

static int parse_bl_mapping_table(struct abc_common *abc, xmlNodePtr curNode)
{
	int i = 0;
	int j = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode->children; //subNode table
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			propNode = subNode->children;
			while (NULL != propNode) {
				attrPtr = propNode->properties;
				while (NULL != attrPtr) {
                	if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"ambient")) {
                    	szPropity = xmlGetProp(propNode, (const xmlChar*)"ambient");
						abc->bl_table[i].item[j].ambient = strtoul((char *)szPropity, (char **)&endptr, 0);
						ENG_LOG("ambient %d \n", abc->bl_table[i].item[j].ambient);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"brightness")) {
						szPropity = xmlGetProp(propNode, (const xmlChar*)"brightness");
						abc->bl_table[i].item[j].backlight = strtoul((char *)szPropity, (char **)&endptr, 0);
						ENG_LOG("brightness %d \n", abc->bl_table[i].item[j].backlight);
					}
					attrPtr = attrPtr->next;
            	}
				j++;
				propNode = propNode->next;
			}
		}
		i++;
		subNode = subNode->next;
	}

	return 0;
}

static int update_slp_mapping_table(struct abc_common *abc, xmlNodePtr curNode)
{
	int i = 0;
	int j = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode->children;
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			propNode = subNode->children;
			while (NULL != propNode) {
				attrPtr = propNode->properties;
				while (NULL != attrPtr) {
                	if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"ambient")) {
						snprintf(numStr, sizeof(numStr), "%d", abc->slp_table[i].item[j].ambient);
						xmlSetProp(propNode, BAD_CAST "ambient", (const xmlChar*)numStr);
						ENG_LOG("ambient %d \n", abc->slp_table[i].item[j].ambient);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"brightness")) {
						snprintf(numStr, sizeof(numStr), "%d", abc->slp_table[i].item[j].slp_brightness_factor);
						xmlSetProp(propNode, BAD_CAST "brightness", (const xmlChar*)numStr);
						ENG_LOG("brightness %d \n", abc->slp_table[i].item[j].slp_brightness_factor);
					}
					attrPtr = attrPtr->next;
            	}
				j++;
				propNode = propNode->next;
			}
		}
		i++;
		subNode = subNode->next;
	}

	return 0;
}

static int update_bl_mapping_table(struct abc_common *abc, xmlNodePtr curNode)
{
	int i = 0;
	int j = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode->children; //subNode table
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			propNode = subNode->children;
			while (NULL != propNode) {
				attrPtr = propNode->properties;
				while (NULL != attrPtr) {
                	if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"ambient")) {
							snprintf(numStr, sizeof(numStr), "%d", abc->bl_table[i].item[j].ambient);
							xmlSetProp(propNode, BAD_CAST "ambient", (const xmlChar*)numStr);
							ENG_LOG("ambient %d \n", abc->bl_table[i].item[j].ambient);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"brightness")) {
						snprintf(numStr, sizeof(numStr), "%d", abc->bl_table[i].item[j].backlight);
						xmlSetProp(propNode, BAD_CAST "brightness", (const xmlChar*)numStr);
						ENG_LOG("brightness %d \n", abc->bl_table[i].item[j].backlight);
					}
					attrPtr = attrPtr->next;
            	}
				j++;
				propNode = propNode->next;
			}
		}
		i++;
		subNode = subNode->next;
	}

	return 0;
}

static int update_slp_config(struct abc_common *abc, xmlNodePtr curNode)
{
	xmlNodePtr propNode;
	xmlChar* szPropity;
	const char *endptr = NULL;
	char numStr[10];

	propNode = curNode->children;
	while (NULL != propNode){
		UPDATE_SLP_CONFIG(brightness);
		UPDATE_SLP_CONFIG(conversion_matrix);
		UPDATE_SLP_CONFIG(brightness_step);
		UPDATE_SLP_CONFIG(second_bright_factor);
		UPDATE_SLP_CONFIG(first_percent_th);
		UPDATE_SLP_CONFIG(first_max_bright_th);
		propNode = propNode->next;
	}

	return 0;

}

int parse_abc_xml(struct abc_common *abc)
{
	xmlDocPtr doc;
	xmlNodePtr curNode;

	doc = xmlReadFile(abc_xml, "utf-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
        ENG_LOG("read %s Document not parsed successfully.\n",__func__);
        return -1;
	}

	curNode = xmlDocGetRootElement(doc);

	if (xmlStrcmp(curNode->name, (const xmlChar*)"root")) {
        ENG_LOG("root node != root\n");
		xmlFreeDoc(doc);
		return -1;
	}
	curNode = curNode->children;

	while(NULL != curNode) {
		if (!xmlStrcmp(curNode->name, (const xmlChar*)"slp_config")) {
			parse_slp_config(abc, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"slp_mapping_table")) {
			parse_slp_mapping_table(abc, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"bl_mapping_table")) {
			parse_bl_mapping_table(abc, curNode);
		}
		curNode = curNode->next;
	}
	xmlSaveFormatFileEnc(abc_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	return 0;

}

int update_abc_xml(struct abc_common *abc)
{
	xmlDocPtr doc;
	xmlNodePtr curNode;

	doc = xmlReadFile(abc_xml, "utf-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
        ENG_LOG("write %s Document not parsed successfully.\n",__func__);
		return -1;
	}
	curNode = xmlDocGetRootElement(doc);
	if (xmlStrcmp(curNode->name, (const xmlChar*)"root")) {
        ENG_LOG("root node != root\n");
		xmlFreeDoc(doc);
        return -1;
	}
	curNode = curNode->children;
	while(NULL != curNode) {
		if (!xmlStrcmp(curNode->name, (const xmlChar*)"slp_config")) {
			update_slp_config(abc, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"slp_mapping_table")) {
			ENG_LOG("curNode name %s \n",curNode->name);
			update_slp_mapping_table(abc, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"bl_mapping_table")) {
			ENG_LOG("curNode name %s \n",curNode->name);
			update_bl_mapping_table(abc, curNode);
		}
		curNode = curNode->next;
	}
	xmlSaveFormatFileEnc(abc_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	return 0;
}
