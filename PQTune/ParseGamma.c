#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <../libxml.h>
#include <utils/Log.h>
#include <PQTuningParmDefine.h>

static int parse_gamma_regs_table(struct gamma_common *gamma, xmlNodePtr curNode)
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
			ENG_LOG("curNode name %s \n",subNode->name);
			propNode = subNode->children;
			while (NULL != propNode) {
				attrPtr = propNode->properties;
				while (NULL != attrPtr) {
                	if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"r")) {
                    	szPropity = xmlGetProp(propNode, (const xmlChar*)"r");
						gamma->gamma.r[i] = strtoul((char *)szPropity, (char **)&endptr, 0);
						ENG_LOG("r %d \n", gamma->gamma.r[i]);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"g")) {
						szPropity = xmlGetProp(propNode, (const xmlChar*)"g");
						gamma->gamma.g[i] = strtoul((char *)szPropity, (char **)&endptr, 0);
						ENG_LOG("g %d \n", gamma->gamma.g[i]);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"b")) {
						szPropity = xmlGetProp(propNode, (const xmlChar*)"b");
						gamma->gamma.b[i] = strtoul((char *)szPropity, (char **)&endptr, 0);
						ENG_LOG("b %d \n", gamma->gamma.b[i]);
					}
					attrPtr = attrPtr->next;
            	}
				i++;
				propNode = propNode->next;
			}
		}
		i = 0;
		subNode = subNode->next;
	}

	return 0;
}

static int update_gamma_regs_table(struct gamma_common *gamma, xmlNodePtr curNode)
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
                			if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"r")) {
						snprintf(numStr, sizeof(numStr), "%d", gamma->gamma.r[i]);
						xmlSetProp(propNode, BAD_CAST "r", (const xmlChar*)numStr);
						ENG_LOG("r %d \n", gamma->gamma.r[i]);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"g")) {
						snprintf(numStr, sizeof(numStr), "%d", gamma->gamma.g[i]);
						xmlSetProp(propNode, BAD_CAST "g", (const xmlChar*)numStr);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"b")) {
						snprintf(numStr, sizeof(numStr), "%d", gamma->gamma.b[i]);
						xmlSetProp(propNode, BAD_CAST "b", (const xmlChar*)numStr);
					}
					attrPtr = attrPtr->next;
            	}
				i++;
				propNode = propNode->next;
			}
		}
		i = 0;
		subNode = subNode->next;
	}

	return 0;
}

int update_gamma_xml(struct gamma_common *gamma)
{

	xmlDocPtr doc;
	xmlNodePtr curNode;

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
	while(NULL != curNode) {
		if (!xmlStrcmp(curNode->name, (const xmlChar*)"gamma_config"))
			update_gamma_regs_table(gamma, curNode);
			curNode = curNode->next;
	}
	xmlSaveFormatFileEnc(gamma_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	return 0;
}

int parse_gamma_xml(struct gamma_common *gamma)
{

	xmlDocPtr doc;
	xmlNodePtr curNode;

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
	while(NULL != curNode) {
		if (!xmlStrcmp(curNode->name, (const xmlChar*)"gamma_config"))
			parse_gamma_regs_table(gamma, curNode);
			curNode = curNode->next;
	}
	xmlSaveFormatFileEnc(gamma_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	ENG_LOG("this is pq\n");
	return 0;
}
