#include <utils/Log.h>
#include "tune_lite_r2p0.h"
#include "pq_xml.h"

static int parse_gamma_version(struct gamma_common *gamma, xmlNodePtr curNode)
{
	xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode; //subNode table
	while(!xmlStrcmp(subNode->name, "enhance")) {
		if(xmlHasProp(subNode, BAD_CAST "version")) {
			szPropity = xmlGetProp(subNode, (const xmlChar*) "version");
			gamma->version.version = strtoul((char *)szPropity, (char **)&endptr, 0);
		} else if (xmlHasProp(subNode, BAD_CAST "major_version")) {
			szPropity = xmlGetProp(subNode, (const xmlChar*) "major_version");
			gamma->nMajorVersion = strtoul((char *)szPropity, (char **)&endptr, 0);
		}
		subNode = subNode->next;
	}
	return 0;
}

static int parse_gamma_child_config(struct gamma_common *gamma, xmlNodePtr curNode , int i)
{
	xmlNodePtr propNode;
	xmlChar* szPropity;
	xmlAttrPtr attrPtr;
	const char *endptr = NULL;
	int j = 0;

	propNode = curNode->children;
	while (NULL != propNode) {
		attrPtr = propNode->properties;
		while (NULL != attrPtr) {
			if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"r")) {
				szPropity = xmlGetProp(propNode, (const xmlChar*)"r");
				gamma->gamma[i].r[j] = strtoul((char *)szPropity, (char **)&endptr, 0); 
				ENG_LOG("r %d \n", gamma->gamma[i].r[j]);
			} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"g")) {
				szPropity = xmlGetProp(propNode, (const xmlChar*)"g");
				gamma->gamma[i].g[j] = strtoul((char *)szPropity, (char **)&endptr, 0); 
			} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"b")) {
				szPropity = xmlGetProp(propNode, (const xmlChar*)"b");
				gamma->gamma[i].b[j] = strtoul((char *)szPropity, (char **)&endptr, 0); 
			}   
			attrPtr = attrPtr->next;
		}   
		j++;
		propNode = propNode->next;
	} 

	return 0;
}   

static int parse_gamma_regs_table(struct gamma_common *gamma, xmlNodePtr curNode)
{
	xmlNodePtr subNode;
	xmlChar* szPropity;

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode; //subNode table
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
			if(!xmlStrcmp(szPropity, (const xmlChar *) "default")) {
				parse_gamma_child_config(gamma, subNode, 0);
			} else if(!xmlStrcmp(szPropity, (const xmlChar *) "standard")) {
				parse_gamma_child_config(gamma, subNode, 1);
			}
		}
		subNode = subNode->next;
	}

	return 0;
}

static int update_gamma_version(struct gamma_common *gamma, xmlNodePtr curNode)
{
    int i = 0;
    xmlNodePtr subNode;
    char numStr[12];

    ENG_LOG("ggaammaa version = %d \n", gamma->version.version);
    subNode = curNode; //subNode table
	while(!xmlStrcmp(subNode->name, "enhance")) {
        if(xmlHasProp(subNode, BAD_CAST "version")) {
            snprintf(numStr, sizeof(numStr), "%d", gamma->version.version);
            xmlSetProp(subNode, BAD_CAST "version", (const xmlChar*)numStr);
        }
		subNode = subNode->next;
	}
    return 0;
}

static int update_gamma_child_config(struct gamma_common *gamma, xmlNodePtr curNode , int i)
{

	int j = 0;
	const char *endptr = NULL;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];

	propNode = curNode->children;
	while (NULL != propNode) {
		attrPtr = propNode->properties;
		while (NULL != attrPtr) {
			if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"r")) {
				snprintf(numStr, sizeof(numStr), "%d", gamma->gamma[i].r[j]);
				xmlSetProp(propNode, BAD_CAST "r", (const xmlChar*)numStr);
				ENG_LOG("r %d \n", gamma->gamma[i].r[j]);
			} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"g")) {
				snprintf(numStr, sizeof(numStr), "%d", gamma->gamma[i].g[j]);
				xmlSetProp(propNode, BAD_CAST "g", (const xmlChar*)numStr);
			} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"b")) {
				snprintf(numStr, sizeof(numStr), "%d", gamma->gamma[i].b[j]);
				xmlSetProp(propNode, BAD_CAST "b", (const xmlChar*)numStr);
			}
			attrPtr = attrPtr->next;
		}
		j++;
		propNode = propNode->next;
	}

	return 0;
}

static int update_gamma_regs_table(struct gamma_common *gamma, xmlNodePtr curNode)
{
	int i = 0;
	xmlNodePtr subNode;
	xmlChar* szPropity;
	char numStr[10];

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode; //subNode table
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			if(xmlHasProp(subNode, BAD_CAST "mode")) {
				ENG_LOG("curNode name %s \n",subNode->name);
				szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
				if(!xmlStrcmp(szPropity, (const xmlChar *) "default")) {
					update_gamma_child_config(gamma, subNode, 0);
				} else if(!xmlStrcmp(szPropity, (const xmlChar *) "standard")) {
					update_gamma_child_config(gamma, subNode, 1);
				}
			}
		}
		i = 0;
		subNode = subNode->next;
	}

	return 0;
}

int update_lite_r2p0_gamma_xml(struct gamma_common *gamma)
{

	xmlDocPtr doc;
	xmlNodePtr curNode;
	xmlNodePtr tmpNode;

	doc = xmlReadFile(gamma_xml, "utf-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
		ENG_LOG("Document not parsed successfully.\n");
		return -1;
	}

	curNode = xmlDocGetRootElement(doc);

	if (xmlStrcmp(curNode->name, (const xmlChar*)"gamma_config"))
	{
		ENG_LOG("gamma_config node != root\n");
		xmlFreeDoc(doc);
		return -1;
	}

	curNode = curNode->children;

	if(tmpNode = FindNode(curNode, "reg_table"))
		update_gamma_regs_table(gamma, tmpNode);
	if(tmpNode = FindNode(curNode, "enhance"))
		update_gamma_version(gamma, tmpNode);

	xmlSaveFormatFileEnc(gamma_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	return 0;
}

int parse_lite_r2p0_gamma_xml(struct gamma_common *gamma)
{

	xmlDocPtr doc;
	xmlNodePtr curNode;
	xmlNodePtr tmpNode;

	doc = xmlReadFile(gamma_xml, "utf-8", XML_PARSE_NOBLANKS);
	if (NULL == doc)
	{
		ENG_LOG("Document not parsed successfully.\n");
		return -1;
	}

	curNode = xmlDocGetRootElement(doc);

	if (xmlStrcmp(curNode->name, (const xmlChar*)"gamma_config")) {
		ENG_LOG("gamma_config node != root\n");
		xmlFreeDoc(doc);
		return -1;
	}

	curNode = curNode->children;

	if(tmpNode = FindNode(curNode, "reg_table"))
		parse_gamma_regs_table(gamma, tmpNode);
	if(tmpNode = FindNode(curNode, "enhance"))
		parse_gamma_version(gamma, tmpNode);

	xmlSaveFormatFileEnc(gamma_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	ENG_LOG("this is pq\n");
	return 0;
}
