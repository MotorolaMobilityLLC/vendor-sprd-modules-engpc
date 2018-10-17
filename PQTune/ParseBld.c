#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <../libxml.h>
#include <utils/Log.h>
#include "PQTuningParmDefine.h"

#define PARSE_BLD_HSVCM(i, X) \
({	\
	if (xmlHasProp(propNode, BAD_CAST #X)) { \
		szPropity = xmlGetProp(propNode, (const xmlChar*) #X); \
		bld->hsvcm[i].cm.X = strtoul((char *)szPropity, (char **)&endptr, 0); \
		ENG_LOG(""#X" %d \n", bld->hsvcm[i].cm.X); \
		propNode = propNode->next; \
	}	\
})

#define UPDATE_BLD_HSVCM(i, X) \
({	\
	if (xmlHasProp(propNode, BAD_CAST #X)) {	\
		snprintf(numStr, sizeof(numStr), "%d", bld->hsvcm[i].cm.X);	\
		xmlSetProp(propNode, BAD_CAST #X, (const xmlChar*)numStr);	\
		ENG_LOG(""#X" %d \n", bld->hsvcm[i].cm.X);	\
		propNode = propNode->next; \
	}	\
})


#define PARSE_CM_HSVCM_ARRAYS(i) \
({ \
	PARSE_BLD_HSVCM(i, coef00); \
	PARSE_BLD_HSVCM(i, coef01); \
	PARSE_BLD_HSVCM(i, coef02); \
	PARSE_BLD_HSVCM(i, coef03); \
	PARSE_BLD_HSVCM(i, coef10); \
	PARSE_BLD_HSVCM(i, coef11); \
	PARSE_BLD_HSVCM(i, coef12); \
	PARSE_BLD_HSVCM(i, coef13); \
	PARSE_BLD_HSVCM(i, coef20); \
	PARSE_BLD_HSVCM(i, coef21); \
	PARSE_BLD_HSVCM(i, coef22); \
	PARSE_BLD_HSVCM(i, coef23); \
})

#define UPDATE_CM_HSVCM_ARRAYS(i) \
({ \
	UPDATE_BLD_HSVCM(i, coef00); \
	UPDATE_BLD_HSVCM(i, coef01); \
	UPDATE_BLD_HSVCM(i, coef02); \
	UPDATE_BLD_HSVCM(i, coef03); \
	UPDATE_BLD_HSVCM(i, coef10); \
	UPDATE_BLD_HSVCM(i, coef11); \
	UPDATE_BLD_HSVCM(i, coef12); \
	UPDATE_BLD_HSVCM(i, coef13); \
	UPDATE_BLD_HSVCM(i, coef20); \
	UPDATE_BLD_HSVCM(i, coef21); \
	UPDATE_BLD_HSVCM(i, coef22); \
	UPDATE_BLD_HSVCM(i, coef23); \
})


static int parse_hsvcm(struct bld_common *bld, xmlNodePtr subNode, int i)
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
				bld->hsvcm[i].hsv.table[j].hue = strtoul((char *)szPropity, (char **)&endptr, 0);
				ENG_LOG("hue %d \n", bld->hsvcm[i].hsv.table[j].hue);
			} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"sat")) {
				szPropity = xmlGetProp(propNode, (const xmlChar*)"sat");
				bld->hsvcm[i].hsv.table[j].sat = strtoul((char *)szPropity, (char **)&endptr, 0);
				ENG_LOG("sat %d \n", bld->hsvcm[i].hsv.table[j].sat);
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

static int parse_bld_version(struct bld_common *bld, xmlNodePtr curNode)
{
        xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;

        ENG_LOG("curNode name %s \n",curNode->name);
        subNode = curNode; //subNode table
        if(xmlHasProp(subNode, BAD_CAST "version")) {
		szPropity = xmlGetProp(subNode, (const xmlChar*) "version");
		bld->version.version = strtoul((char *)szPropity, (char **)&endptr, 0);
	}
        return 0;
}

static int parse_bld_hsvcm(struct bld_common *bld, xmlNodePtr curNode)
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
			if(!xmlStrcmp(szPropity, (const xmlChar *) "default"))
				parse_hsvcm(bld, subNode->children, i);
			else if (!xmlStrcmp(szPropity, (const xmlChar *) "middle"))
				parse_hsvcm(bld, subNode->children, i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "high"))
				parse_hsvcm(bld, subNode->children, i);
		}
		i++;
		subNode = subNode->next;
	}
	return 0;
}

static int update_hsvcm(struct bld_common *bld, xmlNodePtr subNode, int i)
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
				ENG_LOG("hue %d \n", bld->hsvcm[i].hsv.table[j].hue);
				snprintf(numStr, sizeof(numStr), "%d", bld->hsvcm[i].hsv.table[j].hue);
				xmlSetProp(propNode, BAD_CAST "hue", (const xmlChar*)numStr);
			} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"sat")) {
				ENG_LOG("sat %d \n", bld->hsvcm[i].hsv.table[j].sat);
				snprintf(numStr, sizeof(numStr), "%d", bld->hsvcm[i].hsv.table[j].sat);
				xmlSetProp(propNode, BAD_CAST "sat", (const xmlChar*)numStr);
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

static int update_bld_version(struct bld_common *bld, xmlNodePtr curNode)
{
	int i = 0;
	xmlNodePtr subNode;
	char numStr[12];

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode; //subNode table
	if(xmlHasProp(subNode, BAD_CAST "version")) {
		snprintf(numStr, sizeof(numStr), "%d", bld->version.version);
		xmlSetProp(subNode, BAD_CAST "version", (const xmlChar*)numStr);
	}
	return 0;
}

static int update_bld_hsvcm(struct bld_common *bld, xmlNodePtr curNode)
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
			if(!xmlStrcmp(szPropity, (const xmlChar *) "default"))
				update_hsvcm(bld, subNode->children, i);
			else if (!xmlStrcmp(szPropity, (const xmlChar *) "middle"))
				parse_hsvcm(bld, subNode->children, i);
			else if(!xmlStrcmp(szPropity, (const xmlChar *) "high"))
				update_hsvcm(bld, subNode->children, i);
		}
		i++;
		subNode = subNode->next;
	}
	return 0;
}

int parse_bld_xml(struct bld_common *bld)
{
	xmlDocPtr doc;
	xmlNodePtr curNode;

	doc = xmlReadFile(bld_xml, "utf-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
        ENG_LOG("Document not parsed successfully.\n");
        return -1;
	}

	curNode = xmlDocGetRootElement(doc);

	if (xmlStrcmp(curNode->name, (const xmlChar*)"root"))
	{
		ENG_LOG("root node != root\n");
		xmlFreeDoc(doc);
        return -1;
	} else
		ENG_LOG("root node is root\n");

	ENG_LOG("curNode name %s \n",curNode->name);
	curNode = curNode->children;
	while(NULL != curNode) {
		if (!xmlStrcmp(curNode->name, (const xmlChar*)"hsv_cm"))
			parse_bld_hsvcm(bld, curNode);
		else if (!xmlStrcmp(curNode->name, (const xmlChar*)"enhance"))
			 parse_bld_version(bld, curNode);
		curNode = curNode->next;
	}
	xmlSaveFormatFileEnc(bld_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	return 0;

}

int update_bld_xml(struct bld_common *bld)
{
	xmlDocPtr doc;
	xmlNodePtr curNode;

	doc = xmlReadFile(bld_xml, "utf-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
        ENG_LOG("Document not parsed successfully.\n");
        return -1;
	}

	curNode = xmlDocGetRootElement(doc);

	if (xmlStrcmp(curNode->name, (const xmlChar*)"root"))
	{
        ENG_LOG("root node != root\n");
		xmlFreeDoc(doc);
        return -1;
	} else
		ENG_LOG("root node is root\n");

	ENG_LOG("curNode name %s \n",curNode->name);
	curNode = curNode->children;
	while(NULL != curNode) {
		if (!xmlStrcmp(curNode->name, (const xmlChar*)"enhance"))
			update_bld_version(bld, curNode);
		else if (!xmlStrcmp(curNode->name, (const xmlChar*)"hsv_cm"))
			update_bld_hsvcm(bld, curNode);
		curNode = curNode->next;
	}
	xmlSaveFormatFileEnc(bld_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	return 0;

}
