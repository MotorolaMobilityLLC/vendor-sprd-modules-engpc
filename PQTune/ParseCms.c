#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <../libxml.h>
#include <utils/Log.h>
#include "PQTuningParmDefine.h"

#define PARSE_CMS_HSVCM(i, X) \
({	\
	if (xmlHasProp(propNode, BAD_CAST #X)) {	\
		szPropity = xmlGetProp(propNode, (const xmlChar*) #X); \
		cms->hsvcm[i].cm.X = strtoul((char *)szPropity, (char **)&endptr, 0); \
		ENG_LOG(""#X" %d \n", cms->hsvcm[i].cm.X); \
		propNode = propNode->next; \
	}	\
})

#define PARSE_CMS_CM(i, X) \
({	\
	if (xmlHasProp(propNode, BAD_CAST #X)) { \
		szPropity = xmlGetProp(propNode, (const xmlChar*) #X); \
		cms->cm[i].X = strtoul((char *)szPropity, (char **)&endptr, 0);	\
		ENG_LOG(""#X" %d \n", cms->cm[i].X); \
		propNode = propNode->next; \
	} \
})

#define UPDATE_CMS_HSVCM(i, X) \
({	\
	if (xmlHasProp(propNode, BAD_CAST #X)) {	\
		snprintf(numStr, sizeof(numStr), "%d", cms->hsvcm[i].cm.X);	\
		xmlSetProp(propNode, BAD_CAST #X, (const xmlChar*)numStr);	\
		ENG_LOG(""#X" %d \n", cms->hsvcm[i].cm.X);	\
		propNode = propNode->next; \
	}	\
})

#define UPDATE_CMS_CM(i, X) \
({	\
	if (xmlHasProp(propNode, BAD_CAST #X)) {	\
		snprintf(numStr, sizeof(numStr), "%d", cms->cm[i].X);	\
		xmlSetProp(propNode, BAD_CAST #X, (const xmlChar*)numStr);	\
		ENG_LOG(""#X" %d \n", cms->cm[i].X);	\
		propNode = propNode->next; \
	} \
})


#define PARSE_CM_HSVCM_ARRAYS(i) \
({ \
	PARSE_CMS_HSVCM(i, coef00); \
	PARSE_CMS_HSVCM(i, coef01); \
	PARSE_CMS_HSVCM(i, coef02); \
	PARSE_CMS_HSVCM(i, coef03); \
	PARSE_CMS_HSVCM(i, coef10); \
	PARSE_CMS_HSVCM(i, coef11); \
	PARSE_CMS_HSVCM(i, coef12); \
	PARSE_CMS_HSVCM(i, coef13); \
	PARSE_CMS_HSVCM(i, coef20); \
	PARSE_CMS_HSVCM(i, coef21); \
	PARSE_CMS_HSVCM(i, coef22); \
	PARSE_CMS_HSVCM(i, coef23); \
})

#define PARSE_CM_CONFIG_ARRAYS(i) \
({ \
	PARSE_CMS_CM(i, coef00); \
	PARSE_CMS_CM(i, coef01); \
	PARSE_CMS_CM(i, coef02); \
	PARSE_CMS_CM(i, coef03); \
	PARSE_CMS_CM(i ,coef10); \
	PARSE_CMS_CM(i, coef11); \
	PARSE_CMS_CM(i, coef12); \
	PARSE_CMS_CM(i, coef13); \
	PARSE_CMS_CM(i, coef20); \
	PARSE_CMS_CM(i, coef21); \
	PARSE_CMS_CM(i, coef22); \
	PARSE_CMS_CM(i, coef23); \
})


#define UPDATE_CM_CONFIG_ARRAYS(i) \
({ \
	UPDATE_CMS_CM(i, coef00); \
	UPDATE_CMS_CM(i, coef01); \
	UPDATE_CMS_CM(i, coef02); \
	UPDATE_CMS_CM(i, coef03); \
	UPDATE_CMS_CM(i ,coef10); \
	UPDATE_CMS_CM(i, coef11); \
	UPDATE_CMS_CM(i, coef12); \
	UPDATE_CMS_CM(i, coef13); \
	UPDATE_CMS_CM(i, coef20); \
	UPDATE_CMS_CM(i, coef21); \
	UPDATE_CMS_CM(i, coef22); \
	UPDATE_CMS_CM(i, coef23); \
})

#define UPDATE_CM_HSVCM_ARRAYS(i) \
({ \
	UPDATE_CMS_HSVCM(i, coef00); \
	UPDATE_CMS_HSVCM(i, coef01); \
	UPDATE_CMS_HSVCM(i, coef02); \
	UPDATE_CMS_HSVCM(i, coef03); \
	UPDATE_CMS_HSVCM(i, coef10); \
	UPDATE_CMS_HSVCM(i, coef11); \
	UPDATE_CMS_HSVCM(i, coef12); \
	UPDATE_CMS_HSVCM(i, coef13); \
	UPDATE_CMS_HSVCM(i, coef20); \
	UPDATE_CMS_HSVCM(i, coef21); \
	UPDATE_CMS_HSVCM(i, coef22); \
	UPDATE_CMS_HSVCM(i, coef23); \
})


static int parse_hsvcm(struct cms_common *cms, xmlNodePtr subNode, int i)
{
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char *endptr = NULL;
	int j = 0;

	propNode = subNode->children;
	while (NULL != propNode) {
		attrPtr = propNode->properties;
		while (NULL != attrPtr) {
			if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"hue")) {
                szPropity = xmlGetProp(propNode, (const xmlChar*)"hue");
				cms->hsvcm[i].hsv.table[j].hue = strtoul((char *)szPropity, (char **)&endptr, 0);
				ENG_LOG("parse_hue j %d hue %d \n", j, cms->hsvcm[i].hsv.table[j].hue);
			} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"sat")) {
				szPropity = xmlGetProp(propNode, (const xmlChar*)"sat");
				cms->hsvcm[i].hsv.table[j].sat = strtoul((char *)szPropity, (char **)&endptr, 0);
				ENG_LOG("sat %d \n", cms->hsvcm[i].hsv.table[j].sat);
			}
			attrPtr = attrPtr->next;
		}
		j++;
		propNode = propNode->next;
	}

	propNode = subNode->next->children;
	PARSE_CM_HSVCM_ARRAYS(i);

	return 0;
}

static int parse_cms_hsvcm(struct cms_common *cms, xmlNodePtr curNode)
{
	int i = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode->children; //subNode table
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
			if(!xmlStrcmp(szPropity, (const xmlChar *) "auto"))
				parse_hsvcm(cms, subNode->children, i);
			else if (!xmlStrcmp(szPropity, (const xmlChar *) "enhance"))
				parse_hsvcm(cms, subNode->children, i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "standard"))
				parse_hsvcm(cms, subNode->children, i);
		}
		i++;
		subNode = subNode->next;
	}
	return 0;
}

static int parse_cms_rgbmap_table(struct cms_common *cms, xmlNodePtr curNode)
{
	int i = 0;
	int j = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;

	ENG_LOG("curNode name %s \n",curNode->name);
	while(NULL != subNode) {
		ENG_LOG("curNode name %s \n",subNode->name);
		propNode = subNode->children;
		while (NULL != propNode) {
			attrPtr = propNode->properties;
			while (NULL != attrPtr) {
				if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"rgb")) {
					szPropity = xmlGetProp(propNode, (const xmlChar*)"rgb");
					cms->rgbcm[i].rgb = strtoul((char *)szPropity, (char **)&endptr, 0);
					ENG_LOG("rgb %d \n", cms->rgbcm[i].rgb);
				} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"index")) {
					szPropity = xmlGetProp(propNode, (const xmlChar*)"index");
					cms->rgbcm[i].cmindex = strtoul((char *)szPropity, (char **)&endptr, 0);
					ENG_LOG("index %d \n", cms->rgbcm[i].cmindex);
				}
				attrPtr = attrPtr->next;
            		}
			propNode = propNode->next;
		}
	}

	return 0;
}


static int parse_cm_config_table(struct cms_common *cms, xmlNodePtr curNode)
{
	int i = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode->children; //subNode table
	propNode = subNode->children;

	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
			if(!xmlStrcmp(szPropity, (const xmlChar *) "cold"))
				PARSE_CM_CONFIG_ARRAYS(i);
			else if (!xmlStrcmp(szPropity, (const xmlChar *) "warm"))
				PARSE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "00"))
				PARSE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "01"))
				PARSE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "02"))
				PARSE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "03"))
				PARSE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "04"))
				PARSE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "05"))
				PARSE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "06"))
				PARSE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "07"))
				PARSE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "08"))
				PARSE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "09"))
				PARSE_CM_CONFIG_ARRAYS(i);
		}
		i++;
		subNode = subNode->next;
		if(subNode)
			propNode = subNode->children;
	}

	ENG_LOG("cm_config finish0 %s \n",curNode->name);
	return 0;
}

static int update_hsvcm(struct cms_common *cms, xmlNodePtr subNode, int i)
{
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char *endptr = NULL;
	int j = 0;
	char numStr[10];

	propNode = subNode->children;
	while (NULL != propNode) {
		attrPtr = propNode->properties;
		while (NULL != attrPtr) {
			if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"hue")) {
				snprintf(numStr, sizeof(numStr), "%d", cms->hsvcm[i].hsv.table[j].hue);
				//snprintf(numStr, sizeof(numStr), "%d", 100);
				xmlSetProp(propNode, BAD_CAST "hue", (const xmlChar*)numStr);
				ENG_LOG("hue %d \n", cms->hsvcm[i].hsv.table[j].hue);
			} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"sat")) {
				snprintf(numStr, sizeof(numStr), "%d", cms->hsvcm[i].hsv.table[j].sat);
				//snprintf(numStr, sizeof(numStr), "%d", 100);
				xmlSetProp(propNode, BAD_CAST "sat", (const xmlChar*)numStr);
				ENG_LOG("sat %d \n", cms->hsvcm[i].hsv.table[j].sat);
			}
			attrPtr = attrPtr->next;
 		}
		j++;
		propNode = propNode->next;
	}
	propNode = subNode->next->children;
	UPDATE_CM_HSVCM_ARRAYS(i);

	return 0;
}

int update_cms_hsvcm(struct cms_common *cms, xmlNodePtr curNode)
{
	int i = 0;
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
			szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
			if(!xmlStrcmp(szPropity, (const xmlChar *) "auto"))
				update_hsvcm(cms, subNode->children, i);
			else if (!xmlStrcmp(szPropity, (const xmlChar *) "enhance"))
				update_hsvcm(cms, subNode->children, i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "standard"))
				update_hsvcm(cms, subNode->children, i);
		}
		i++;
		subNode = subNode->next;
	}
	return 0;
}

static int update_cms_rgbmap_table(struct cms_common *cms, xmlNodePtr curNode)
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
	while(NULL != subNode) {
		ENG_LOG("curNode name %s \n",subNode->name);
		propNode = subNode->children;
		while (NULL != propNode) {
			attrPtr = propNode->properties;
			while (NULL != attrPtr) {
 				if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"rgb")) {
					snprintf(numStr, sizeof(numStr), "%d", cms->rgbcm[i].rgb);
					xmlSetProp(propNode, BAD_CAST "rgb", (const xmlChar*)numStr);
					ENG_LOG("rgb %d \n", cms->rgbcm[i].rgb);
				} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"index")) {
					snprintf(numStr, sizeof(numStr), "%d", cms->rgbcm[i].cmindex);
					xmlSetProp(propNode, BAD_CAST "index", (const xmlChar*)numStr);
					ENG_LOG("index %d \n", cms->rgbcm[i].cmindex);
				}
				attrPtr = attrPtr->next;
            }
			i++;
			propNode = propNode->next;
		}
	}

	return 0; 
}


static int update_cm_config_table(struct cms_common *cms, xmlNodePtr curNode)
{
	int i = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode->children; //subNode table
	propNode = subNode->children;

	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
			if(!xmlStrcmp(szPropity, (const xmlChar *) "cold"))
				UPDATE_CM_CONFIG_ARRAYS(i);
			else if (!xmlStrcmp(szPropity, (const xmlChar *) "warm"))
				UPDATE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "00"))
				PARSE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "01"))
				UPDATE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "02"))
				UPDATE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "03"))
				UPDATE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "04"))
				UPDATE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "05"))
				UPDATE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "06"))
				UPDATE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "07"))
				UPDATE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "08"))
				UPDATE_CM_CONFIG_ARRAYS(i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "09"))
				UPDATE_CM_CONFIG_ARRAYS(i);
		}
		i++;
		subNode = subNode->next;
		if(subNode)
			propNode = subNode->children;
	}

	ENG_LOG("cm_config finish0 %s \n",curNode->name);
	return 0;
}

int update_cms_xml(struct cms_common *cms)
{
	xmlDocPtr doc;
	xmlNodePtr curNode;

	doc = xmlReadFile(cms_xml, "utf-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
        ENG_LOG("Document not parsed successfully.\n");
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
		if (!xmlStrcmp(curNode->name, (const xmlChar*)"hsv_cm")) {
			update_cms_hsvcm(cms, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"rgb_map")) {
			ENG_LOG("curNode name %s \n",curNode->name);
			update_cms_rgbmap_table(cms, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"cm_cfg")) {
			ENG_LOG("curNode name %s \n",curNode->name);
			update_cm_config_table(cms, curNode);
		}
		curNode = curNode->next;
	}
	xmlSaveFormatFileEnc(cms_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	ENG_LOG("this is pq\n");
	return 0;
}


int parse_cms_xml(struct cms_common *cms)
{
	xmlDocPtr doc;
	xmlNodePtr curNode;

	doc = xmlReadFile(cms_xml, "utf-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
        ENG_LOG("Document not parsed successfully.\n");
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
		if (!xmlStrcmp(curNode->name, (const xmlChar*)"hsv_cm")) {
			parse_cms_hsvcm(cms, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"rgb_map")) {
			ENG_LOG("curNode name %s \n",curNode->name);
			parse_cms_rgbmap_table(cms, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"cm_cfg")) {
			ENG_LOG("curNode name %s \n",curNode->name);
			parse_cm_config_table(cms, curNode);
		}
		curNode = curNode->next;
	}
	xmlSaveFormatFileEnc(cms_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	ENG_LOG("this is pq\n");
	return 0;
}
