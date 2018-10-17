#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <../libxml.h>
#include <utils/Log.h>
#include "PQTuningParmDefine.h"

#define PARSE_EPF_CONFIG(X, I, J) \
	({ \
	 if (xmlHasProp(propNode, BAD_CAST #X)) { \
	 szPropity = xmlGetProp(propNode, (const xmlChar*) #X); \
	 abc->sceneTable[I].sceneTableItem[J].epfCfg.X = strtoul((char *)szPropity, (char **)&endptr, 0); \
	 ENG_LOG("aabbcc "#X" = %d \n",abc->sceneTable[I].sceneTableItem[J].epfCfg.X);\
	 propNode = propNode->next; \
	 } \
	 })

#define PARSE_SLP_CONFIG(X, I, J) \
	({ \
	 if (xmlHasProp(propNode, BAD_CAST #X)) { \
	 szPropity = xmlGetProp(propNode, (const xmlChar*) #X); \
	 abc->sceneTable[I].sceneTableItem[J].slpCfg.X = strtoul((char *)szPropity, (char **)&endptr, 0); \
	 propNode = propNode->next; \
	 } \
	 })

#define UPDATE_EPF_CONFIG(X, I, J)	\
	({	\
	 if (xmlHasProp(propNode, BAD_CAST #X)) { \
	 snprintf(numStr, sizeof(numStr), "%d", abc->sceneTable[I].sceneTableItem[J].epfCfg.X); \
	 xmlSetProp(propNode, BAD_CAST #X, (const xmlChar*)numStr); \
	 propNode = propNode->next; \
	 } \
	 })


#define UPDATE_SLP_CONFIG(X, I, J) \
	({	\
	 if (xmlHasProp(propNode, BAD_CAST #X)) { \
	 snprintf(numStr, sizeof(numStr), "%d", abc->sceneTable[I].sceneTableItem[J].slpCfg.X); \
	 xmlSetProp(propNode, BAD_CAST #X, (const xmlChar*)numStr); \
	 propNode = propNode->next; \
	 } \
	 })

#define UPDATE_SLP_CONFIG_CHILD_XML(X) \
	({ \
	childNode = xmlNewNode(NULL,BAD_CAST "param"); \
	if(!propNode) \
		xmlAddChild(subNode, childNode); \
	snprintf(numStr, sizeof(numStr), "%d", 1); \
	xmlNewProp(childNode, BAD_CAST #X, BAD_CAST numStr); \
	if (propNode) \
		xmlAddSibling(propNode, childNode); \
	propNode = childNode; \
	 })

#define UPDATE_EPF_CONFIG_CHILD_XML(X) \
	({ \
	childNode = xmlNewNode(NULL,BAD_CAST "param"); \
	if(!propNode) \
		xmlAddChild(subNode, childNode); \
	snprintf(numStr, sizeof(numStr), "%d", 1); \
	xmlNewProp(childNode, BAD_CAST #X, BAD_CAST numStr);	\
	if (propNode) \
		xmlAddSibling(propNode, childNode); \
	propNode = childNode; \
	 })

static void RemoveNode(xmlNode *ParentNode, xmlNode *ChildNode)
{
	if (ChildNode == NULL) {
		ENG_LOG("error: ChildNode is null");
		return;
	}

	xmlNodePtr siblingNode = ChildNode->next;//next index

	while (siblingNode != NULL) {
		if (siblingNode->type == XML_ELEMENT_NODE) {
			ENG_LOG("PQ debug: found sibling: %s\n", siblingNode->name);
			break;
		}
		siblingNode = siblingNode->next;
	}

	ENG_LOG("PQ debug: ParentNode: %s, ChildNode: %s\n", ParentNode->name, ChildNode->name);

	xmlNode *childrenNode = ChildNode->children;//item
	if (childrenNode == NULL) {
		ENG_LOG("PQ warn: childrenNode is null\n");
	}
	xmlNodePtr nextChildNode = NULL;

	while (childrenNode != NULL) {
		ENG_LOG("PQ debug: childrenNode: %s\n", childrenNode->name);
		nextChildNode = childrenNode->next;//next item
		xmlUnlinkNode(childrenNode);
		childrenNode = nextChildNode;
	}
	xmlUnlinkNode(ChildNode);
	xmlFreeNode(ChildNode);
}

static int parse_epf_child_config(struct abc_common *abc, xmlNodePtr curNode , int i)
{
	xmlNodePtr propNode;
	xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;
	int j = 0;

	subNode = curNode->children;//number
	while(NULL != subNode) {
		propNode = subNode->children;//param
		while (NULL != propNode) {
			PARSE_EPF_CONFIG(epsilon0, i, j);
			PARSE_EPF_CONFIG(epsilon1, i, j);
			PARSE_EPF_CONFIG(gain0, i, j);
			PARSE_EPF_CONFIG(gain1, i, j);
			PARSE_EPF_CONFIG(gain2, i, j);
			PARSE_EPF_CONFIG(gain3, i, j);
			PARSE_EPF_CONFIG(gain4, i, j);
			PARSE_EPF_CONFIG(gain5, i, j);
			PARSE_EPF_CONFIG(gain6, i, j);
			PARSE_EPF_CONFIG(gain7, i, j);
			PARSE_EPF_CONFIG(max_diff, i, j);
			PARSE_EPF_CONFIG(min_diff, i, j);
			if (propNode)
				propNode = propNode->next;
			ENG_LOG("aabbcc parse %s  j = %d \n", __func__, j);
		}
		subNode = subNode->next;
		j++;
	}
	return 0;
}

static int parse_epf_config(struct abc_common *abc, xmlNodePtr curNode)
{
	xmlNodePtr propNode;
	xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;

	subNode = curNode->children;//config mode
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
			if(!xmlStrcmp(szPropity, (const xmlChar *) "normal")) {
				parse_epf_child_config(abc, subNode, 0);
			} else if(!xmlStrcmp(szPropity, (const xmlChar *) "low power")) {
				parse_epf_child_config(abc, subNode, 1);
			}
		}
		subNode = subNode->next;
	}

	return 0;
}

static int parse_slp_child_config(struct abc_common *abc, xmlNodePtr curNode, int i)
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
			PARSE_SLP_CONFIG(brightness, i, j);
			PARSE_SLP_CONFIG(conversion_matrix, i, j);
			PARSE_SLP_CONFIG(brightness_step, i, j);
			PARSE_SLP_CONFIG(second_bright_factor, i, j);
			PARSE_SLP_CONFIG(first_percent_th, i, j);
			PARSE_SLP_CONFIG(first_max_bright_th, i, j);
			if (propNode)
				propNode = propNode->next;
		}
		subNode = subNode->next;
		j++;
	}
	return 0;

}

static int parse_slp_config(struct abc_common *abc, xmlNodePtr curNode)
{
	xmlNodePtr propNode;
	xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;

	subNode = curNode->children;
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
			if(!xmlStrcmp(szPropity, (const xmlChar *) "normal")) {
				parse_slp_child_config(abc, subNode, 0);
			} else if(!xmlStrcmp(szPropity, (const xmlChar *) "low power")) {
				parse_slp_child_config(abc, subNode, 1);
			}
		}
		subNode = subNode->next;
	}

	return 0;
}

static int parse_abc_version(struct abc_common *abc, xmlNodePtr curNode)
{
	xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode; //enhance version
	if(xmlHasProp(subNode, BAD_CAST "version")) {
		szPropity = xmlGetProp(subNode, (const xmlChar*) "version");
		abc->version.version = strtoul((char *)szPropity, (char **)&endptr, 0);
		ENG_LOG("aaaaa abc verison %d\n", abc->version.version);
	}
	return 0;
}

static int parse_slp_mapping_child_table(struct abc_common *abc, xmlNodePtr curNode, int i)
{
	int j = 0;
	int k = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;

	subNode = curNode->children;//number index
	while(NULL != subNode) {
		propNode = subNode->children;//item
		while (NULL != propNode) {
			attrPtr = propNode->properties;
			while (NULL != attrPtr) {
				if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"ambient")) {
					szPropity = xmlGetProp(propNode, (const xmlChar*)"ambient");
					abc->sceneTable[i].sceneTableItem[j].slpMappingTable.item[k].ambient = strtoul((char *)szPropity, (char **)&endptr, 0);
				} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"brightness")) {
					szPropity = xmlGetProp(propNode, (const xmlChar*)"brightness");
					abc->sceneTable[i].sceneTableItem[j].slpMappingTable.item[k].slp_brightness_factor = strtoul((char *)szPropity, (char **)&endptr, 0);
				}
				attrPtr = attrPtr->next;
			}
			k++;
			propNode = propNode->next;
		}
		abc->sceneTable[i].sceneTableItem[j].slpMappingTable.map_num = k;
		j++;
		k = 0;
		subNode = subNode->next;
	}

	abc->sceneTable[i].num = j;

	return 0;
}

static int parse_slp_mapping_table(struct abc_common *abc, xmlNodePtr curNode)
{
	xmlNodePtr propNode;
	xmlNodePtr subNode;
	xmlChar* szPropity;
	const char *endptr = NULL;

	subNode = curNode->children;// table mode
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
			if(!xmlStrcmp(szPropity, (const xmlChar *) "normal")) {
				parse_slp_mapping_child_table(abc, subNode, 0);
			} else if(!xmlStrcmp(szPropity, (const xmlChar *) "low power")) {
				parse_slp_mapping_child_table(abc, subNode, 1);
			}
		}
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
	subNode = curNode->children; //table mode
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			propNode = subNode->children;
			while (NULL != propNode) {
				attrPtr = propNode->properties;
				while (NULL != attrPtr) {
					if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"ambient")) {
						szPropity = xmlGetProp(propNode, (const xmlChar*)"ambient");
						abc->blMappingTable[i].item[j].ambient = strtoul((char *)szPropity, (char **)&endptr, 0);
					} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"brightness")) {
						szPropity = xmlGetProp(propNode, (const xmlChar*)"brightness");
						abc->blMappingTable[i].item[j].backlight = strtoul((char *)szPropity, (char **)&endptr, 0);
					}
					attrPtr = attrPtr->next;
				}
				j++;
				propNode = propNode->next;
			}
			abc->blMappingTable[i].map_num = j;
		}
		i++;
		j = 0;
		subNode = subNode->next;
	}

	return 0;
}

static int update_abc_version(struct abc_common *abc, xmlNodePtr curNode)
{
	int i = 0;
	xmlNodePtr subNode;
	char numStr[12];

	ENG_LOG("curNode name %s \n",curNode->name);
	subNode = curNode; //enhance version
	if(xmlHasProp(subNode, BAD_CAST "version")) {
		snprintf(numStr, sizeof(numStr), "%d", abc->version.version);
		xmlSetProp(subNode, BAD_CAST "version", (const xmlChar*)numStr);
	}
	return 0;
}

static int update_slp_items_xml(xmlNodePtr curNode, int wrItemnums, int Itemnums)
{

	xmlNodePtr propNode;
	xmlNodePtr NextpropNode;
	xmlNodePtr childNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];

	ENG_LOG("vvvvvvitem wrItemnums =  %d, Itemnums = %d \n", wrItemnums, Itemnums);
	propNode = curNode;
	if (Itemnums < wrItemnums) {
		for(; Itemnums < wrItemnums; Itemnums++) {
			childNode = xmlNewNode(NULL,BAD_CAST "item");
			snprintf(numStr, sizeof(numStr), "%d", 1);
			xmlNewProp(childNode, BAD_CAST "ambient", BAD_CAST numStr);
			xmlNewProp(childNode,BAD_CAST "brightness", BAD_CAST numStr);
			xmlAddSibling(propNode, childNode);
			propNode = childNode;
		}
	} else {
		propNode = propNode->next;
		while (NULL != propNode) {
			NextpropNode = propNode->next;
			if(propNode)
				RemoveNode(curNode, propNode);
			propNode = NextpropNode;
		}
	}

	return 0;
}

static int update_slp_mapping_items_child_xml(xmlNodePtr curNode, int wrItemnums, int Itemnums)
{

	xmlNodePtr subNode;
	xmlNodePtr propNode = NULL;
	xmlNodePtr NextpropNode;
	xmlNodePtr childNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];

	ENG_LOG("vvvvvvitem wrItemnums =  %d, Itemnums = %d \n", wrItemnums, Itemnums);
	subNode = curNode;
	if (Itemnums < wrItemnums) {
		for(; Itemnums < wrItemnums; Itemnums++) {
			childNode = xmlNewNode(NULL,BAD_CAST "item");
			if(!propNode)
				xmlAddChild(subNode, childNode);
			snprintf(numStr, sizeof(numStr), "%d", 1);
			xmlNewProp(childNode, BAD_CAST "ambient", BAD_CAST numStr);
			xmlNewProp(childNode,BAD_CAST "brightness", BAD_CAST numStr);
			if (propNode)
				xmlAddSibling(propNode, childNode);
			propNode = childNode;
		}
	} else {
		if(propNode)
			propNode = propNode->next;
		while (NULL != propNode) {
			NextpropNode = propNode->next;
			if(propNode)
				RemoveNode(curNode, propNode);
			propNode = NextpropNode;
		}
	}

	return 0;
}

static int update_slp_index_xml(struct abc_common *abc, xmlNodePtr curNode, int Indexnums, int i)
{

	xmlNodePtr subNode;
	xmlNodePtr NextpropNode;
	xmlNodePtr childNode;
	xmlNodePtr parentNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];
	int wrIndexnums, wrItemnums;

	wrIndexnums = abc->sceneTable[i].num;
	subNode = curNode;
	parentNode = curNode->parent;

	if (Indexnums < wrIndexnums) {
		for(; Indexnums < wrIndexnums; Indexnums++) {
			childNode = xmlNewNode(NULL,BAD_CAST "number");
			xmlAddSibling(subNode, childNode);
			snprintf(numStr, sizeof(numStr), "%d", Indexnums);
			xmlNewProp(childNode, BAD_CAST "index", BAD_CAST numStr);
			wrItemnums = abc->sceneTable[i].sceneTableItem[Indexnums].slpMappingTable.map_num;
			update_slp_mapping_items_child_xml(childNode, wrItemnums, 0);
		}
	} else {
		subNode = subNode->next;
		while (NULL != subNode) {
			NextpropNode = subNode->next;
			if(subNode) {
				RemoveNode(subNode, subNode->children);
				RemoveNode(parentNode, subNode);
			}
			subNode = NextpropNode;
		}
	}

	return 0;
}

static int update_slp_mapping_indexs_arrays(struct abc_common *abc, xmlNodePtr curNode, int i)
{

	xmlNodePtr propNode;
	xmlNodePtr subNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	int wrIndexnums, wrItemnums;
	int Indexnums, Itemnums;
	char numStr[10];
	struct slp_mapping_table_item *item;

	ENG_LOG("vvvvvv %s in \n", __func__);
	wrIndexnums = abc->sceneTable[i].num;
	subNode = curNode->children;//number index
	Indexnums = 0;
	Itemnums = 0;
	while(NULL != subNode) {
		propNode = subNode->children;//item
		wrItemnums = abc->sceneTable[i].sceneTableItem[Indexnums].slpMappingTable.map_num;
		while(NULL != propNode) {
			Itemnums++;
			if ((propNode->next) && (Itemnums < wrItemnums))
				propNode = propNode->next;
			else {
				update_slp_items_xml(propNode, wrItemnums, Itemnums);
				break;
			}
		}
		Itemnums = 0;
		Indexnums++;
		if((subNode->next) && (Indexnums < wrIndexnums))
			subNode = subNode->next;
		else {
			update_slp_index_xml(abc, subNode, Indexnums, i);
			break;
		}
	}
	ENG_LOG("vvvvvv %s out \n", __func__);
	return 0;
}

static int update_slp_mapping_child_table(struct abc_common *abc, xmlNodePtr curNode, int i)
{
	xmlNodePtr propNode;
	xmlNodePtr subNode;
	xmlNodePtr NextpropNode;
	xmlNodePtr childNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	int j = 0;
	int k = 0;
	int index_nums, item_nums;
	char numStr[10];

	ENG_LOG("vvvvvv %s in \n", __func__);
	index_nums = abc->sceneTable[i].num;
	subNode = curNode->children;//number index
	while(NULL != subNode) {
		propNode = subNode->children;//item
		while (NULL != propNode) {
			attrPtr = propNode->properties;
			while (NULL != attrPtr) {
				if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"ambient")) {
					snprintf(numStr, sizeof(numStr), "%d", abc->sceneTable[i].sceneTableItem[j].slpMappingTable.item[k].ambient);
					xmlSetProp(propNode, BAD_CAST "ambient", (const xmlChar*)numStr);
				} else if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"brightness")) {
					snprintf(numStr, sizeof(numStr), "%d", abc->sceneTable[i].sceneTableItem[j].slpMappingTable.item[k].slp_brightness_factor);
					xmlSetProp(propNode, BAD_CAST "brightness", (const xmlChar*)numStr);
				}
				attrPtr = attrPtr->next;
			}
			k++;
			propNode = propNode->next;
		}
		k = 0;
		j++;
		subNode = subNode->next;
	}
	ENG_LOG("vvvvvv %s out \n", __func__);
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

	ENG_LOG("vvvvvv update_slp_mapping_table in \n");
	subNode = curNode->children;//table normal
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
			if(!xmlStrcmp(szPropity, (const xmlChar *) "normal")) {
				ENG_LOG("PQ  bl mapping normal \n");
				if(abc->mode == 1) {
					update_slp_mapping_indexs_arrays(abc, subNode, 0);
					update_slp_mapping_child_table(abc, subNode, 0);
				}
			} else if(!xmlStrcmp(szPropity, (const xmlChar *) "low power")) {
				if(abc->mode == 2) {
					update_slp_mapping_indexs_arrays(abc, subNode, 1);
					update_slp_mapping_child_table(abc, subNode, 1);
				}
			}
		}
		subNode = subNode->next;
	}

	ENG_LOG("vvvvvv update_slp_mapping_table out \n");
	return 0;
}

static int update_bl_child_table(struct abc_common *abc, xmlNodePtr curNode, int i)
{
	xmlNodePtr propNode;
	xmlNodePtr NextpropNode;
	xmlNodePtr childNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	int j = 0;
	int nums;
	char numStr[10];

	propNode = curNode->children;
	while (NULL != propNode) {
		nums = abc->blMappingTable[i].map_num;
		attrPtr = propNode->properties;
		while (NULL != attrPtr) {
			if (!xmlStrcmp(attrPtr->name, (const xmlChar*)"ambient")) {
				snprintf(numStr, sizeof(numStr), "%d", abc->blMappingTable[i].item[j].ambient);
				xmlSetProp(propNode, BAD_CAST "ambient", (const xmlChar*)numStr);
			} else if(!xmlStrcmp(attrPtr->name, (const xmlChar*)"brightness")) {
				snprintf(numStr, sizeof(numStr), "%d", abc->blMappingTable[i].item[j].backlight);
				xmlSetProp(propNode, BAD_CAST "brightness", (const xmlChar*)numStr);
			}
			attrPtr = attrPtr->next;
		}
		j++;
		if ((propNode->next) && ( j < nums)) {
			ENG_LOG("PQ nums =  %d j = %d \n", nums, j);
			propNode = propNode->next;
		}
		else
			break;
	}
	if(j < nums) {
		for(; j < nums; j++) {
			childNode = xmlNewNode(NULL, BAD_CAST "item");
			snprintf(numStr, sizeof(numStr), "%d", abc->blMappingTable[i].item[j].ambient);
			xmlNewProp(childNode, BAD_CAST "ambient", BAD_CAST numStr);
			snprintf(numStr, sizeof(numStr), "%d", abc->blMappingTable[i].item[j].backlight);
			xmlNewProp(childNode, BAD_CAST "brightness", BAD_CAST numStr);
			xmlAddSibling(propNode, childNode);
			propNode = childNode;
		}
	} else {
		propNode = propNode->next;
		while (NULL != propNode) {
			NextpropNode = propNode->next;
			if(propNode)
				RemoveNode(curNode, propNode);
			j++;
			propNode = NextpropNode;
			ENG_LOG("PQ REMOVE  j = %d\n", j);
		}
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

	ENG_LOG("PQ curNode name %s abc->mode %d \n", curNode->name, abc->mode);
	subNode = curNode->children; //subNode table
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
			if(!xmlStrcmp(szPropity, (const xmlChar *) "normal")) {
				if(abc->mode == 1)
					update_bl_child_table(abc, subNode, 0);
			} else if(!xmlStrcmp(szPropity, (const xmlChar *) "low power")) {
				if(abc->mode == 2)
					update_bl_child_table(abc, subNode, 1);
			}
		}
		subNode = subNode->next;
	}

	return 0;
}

static int update_epf_child_config(struct abc_common *abc, xmlNodePtr curNode, int i)
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
			UPDATE_EPF_CONFIG(epsilon0, i, j);
			UPDATE_EPF_CONFIG(epsilon1, i, j);
			UPDATE_EPF_CONFIG(gain0, i, j);
			UPDATE_EPF_CONFIG(gain1, i, j);
			UPDATE_EPF_CONFIG(gain2, i, j);
			UPDATE_EPF_CONFIG(gain3, i, j);
			UPDATE_EPF_CONFIG(gain4, i, j);
			UPDATE_EPF_CONFIG(gain5, i, j);
			UPDATE_EPF_CONFIG(gain6, i, j);
			UPDATE_EPF_CONFIG(gain7, i, j);
			UPDATE_EPF_CONFIG(max_diff, i, j);
			UPDATE_EPF_CONFIG(min_diff, i, j);
		}
		j++;
		subNode = subNode->next;
	}

	return 0;
}

#if 1

static int update_epf_config_child_xml(xmlNodePtr curNode)
{

	xmlNodePtr subNode;
	xmlNodePtr propNode = NULL;
	xmlNodePtr NextpropNode;
	xmlNodePtr childNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];

	subNode = curNode;
	UPDATE_EPF_CONFIG_CHILD_XML(epsilon0);
	UPDATE_EPF_CONFIG_CHILD_XML(epsilon1);
	UPDATE_EPF_CONFIG_CHILD_XML(gain0);
	UPDATE_EPF_CONFIG_CHILD_XML(gain1);
	UPDATE_EPF_CONFIG_CHILD_XML(gain2);
	UPDATE_EPF_CONFIG_CHILD_XML(gain3);
	UPDATE_EPF_CONFIG_CHILD_XML(gain4);
	UPDATE_EPF_CONFIG_CHILD_XML(gain5);
	UPDATE_EPF_CONFIG_CHILD_XML(gain6);
	UPDATE_EPF_CONFIG_CHILD_XML(gain7);
	UPDATE_EPF_CONFIG_CHILD_XML(max_diff);
	UPDATE_EPF_CONFIG_CHILD_XML(min_diff);

	return 0;
}

static int update_epf_config_index_xml(struct abc_common *abc, xmlNodePtr curNode, int Indexnums, int i)
{

	xmlNodePtr subNode;
	xmlNodePtr NextpropNode;
	xmlNodePtr childNode;
	xmlNodePtr parentNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];
	int wrIndexnums, wrItemnums;

	wrIndexnums = abc->sceneTable[i].num;
	subNode = curNode;
	parentNode = curNode->parent;

	if (Indexnums < wrIndexnums) {
		for(; Indexnums < wrIndexnums; Indexnums++) {
			childNode = xmlNewNode(NULL,BAD_CAST "number");
			xmlAddSibling(subNode, childNode);
			snprintf(numStr, sizeof(numStr), "%d", Indexnums);
			xmlNewProp(childNode, BAD_CAST "index", BAD_CAST numStr);
			update_epf_config_child_xml(childNode);
		}
	} else {
		subNode = subNode->next;
		while (NULL != subNode) {
			NextpropNode = subNode->next;
			if(subNode) {
				RemoveNode(subNode, subNode->children);
				RemoveNode(parentNode, subNode);
			}
			subNode = NextpropNode;
		}
	}

	return 0;
}

static int update_epf_config_indexs_arrays(struct abc_common *abc, xmlNodePtr curNode, int i)
{

	xmlNodePtr propNode;
	xmlNodePtr subNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	int wrIndexnums, wrItemnums;
	int Indexnums, Itemnums;
	char numStr[10];
	struct slp_mapping_table_item *item;

	ENG_LOG("vvvvvv %s in \n", __func__);
	wrIndexnums = abc->sceneTable[i].num;
	subNode = curNode->children;//number index
	Indexnums = 0;
	while(NULL != subNode) {
		Indexnums++;
		if((subNode->next) && (Indexnums < wrIndexnums))
			subNode = subNode->next;
		else {
			update_epf_config_index_xml(abc, subNode, Indexnums, i);
			break;
		}
	}
	ENG_LOG("vvvvvv %s out \n", __func__);
	return 0;
}
#endif

static int update_epf_config(struct abc_common *abc, xmlNodePtr curNode)
{
	int i = 0;
	int j = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];

	ENG_LOG("PQ curNode name %s \n",curNode->name);
	subNode = curNode->children;//config mode
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
			if(!xmlStrcmp(szPropity, (const xmlChar *) "normal")) {
				ENG_LOG("PQ  epf config normal \n");
				if(abc->mode == 1) {
					update_epf_config_indexs_arrays(abc, subNode, 0);
					update_epf_child_config(abc, subNode, 0);
				}
			} else if(!xmlStrcmp(szPropity, (const xmlChar *) "low power")) {
				if(abc->mode == 2) {
					update_epf_config_indexs_arrays(abc, subNode, 1);
					update_epf_child_config(abc, subNode, 1);
				}
			}
		}
		subNode = subNode->next;
	}

	return 0;
}

static int update_slp_child_config(struct abc_common *abc, xmlNodePtr curNode, int i)
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
			UPDATE_SLP_CONFIG(brightness, i , j);
			UPDATE_SLP_CONFIG(conversion_matrix, i, j);
			UPDATE_SLP_CONFIG(brightness_step, i, j);
			UPDATE_SLP_CONFIG(second_bright_factor, i, j);
			UPDATE_SLP_CONFIG(first_percent_th, i, j);
			UPDATE_SLP_CONFIG(first_max_bright_th, i, j);
		}
		subNode = subNode->next;
		j++;
	}
	return 0;
}

#if 1

static int update_slp_config_child_xml(xmlNodePtr curNode)
{

	xmlNodePtr subNode;
	xmlNodePtr propNode = NULL;
	xmlNodePtr NextpropNode;
	xmlNodePtr childNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];

	subNode = curNode;

	UPDATE_SLP_CONFIG_CHILD_XML(brightness);
	UPDATE_SLP_CONFIG_CHILD_XML(conversion_matrix);
	UPDATE_SLP_CONFIG_CHILD_XML(conversion_matrix);
	UPDATE_SLP_CONFIG_CHILD_XML(brightness_step);
	UPDATE_SLP_CONFIG_CHILD_XML(second_bright_factor);
	UPDATE_SLP_CONFIG_CHILD_XML(first_percent_th);
	UPDATE_SLP_CONFIG_CHILD_XML(first_max_bright_th);

	return 0;
}

static int update_slp_config_index_xml(struct abc_common *abc, xmlNodePtr curNode, int Indexnums, int i)
{

	xmlNodePtr subNode;
	xmlNodePtr NextpropNode;
	xmlNodePtr childNode;
	xmlNodePtr parentNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];
	int wrIndexnums, wrItemnums;

	wrIndexnums = abc->sceneTable[i].num;
	subNode = curNode;
	parentNode = curNode->parent;

	if (Indexnums < wrIndexnums) {
		for(; Indexnums < wrIndexnums; Indexnums++) {
			childNode = xmlNewNode(NULL,BAD_CAST "number");
			xmlAddSibling(subNode, childNode);
			snprintf(numStr, sizeof(numStr), "%d", Indexnums);
			xmlNewProp(childNode, BAD_CAST "index", BAD_CAST numStr);
			update_slp_config_child_xml(childNode);
		}
	} else {
		subNode = subNode->next;
		while (NULL != subNode) {
			NextpropNode = subNode->next;
			if(subNode) {
				RemoveNode(subNode, subNode->children);
				RemoveNode(parentNode, subNode);
			}
			subNode = NextpropNode;
		}
	}

	return 0;
}

static int update_slp_config_indexs_arrays(struct abc_common *abc, xmlNodePtr curNode, int i)
{

	xmlNodePtr propNode;
	xmlNodePtr subNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	int wrIndexnums, wrItemnums;
	int Indexnums, Itemnums;
	char numStr[10];
	struct slp_mapping_table_item *item;

	ENG_LOG("vvvvvv %s in \n", __func__);
	wrIndexnums = abc->sceneTable[i].num;
	subNode = curNode->children;//number index
	Indexnums = 0;
	while(NULL != subNode) {
		Indexnums++;
		if((subNode->next) && (Indexnums < wrIndexnums))
			subNode = subNode->next;
		else {
			update_slp_config_index_xml(abc, subNode, Indexnums, i);
			break;
		}
	}
	ENG_LOG("vvvvvv %s out \n", __func__);
	return 0;
}
#endif

static int update_slp_config(struct abc_common *abc, xmlNodePtr curNode)
{
	int i = 0;
	int j = 0;
	const char *endptr = NULL;
	xmlNodePtr subNode;
	xmlNodePtr propNode;
	xmlAttrPtr attrPtr;
	xmlChar* szPropity;
	char numStr[10];

	ENG_LOG("PQ curNode name %s \n",curNode->name);
	subNode = curNode->children;
	while(NULL != subNode) {
		if(xmlHasProp(subNode, BAD_CAST "mode")) {
			ENG_LOG("curNode name %s \n",subNode->name);
			szPropity = xmlGetProp(subNode, (const xmlChar*)"mode");
			if(!xmlStrcmp(szPropity, (const xmlChar *) "normal")) {
				ENG_LOG("PQ  slp config normal \n");
				if(abc->mode == 1) {
					update_slp_config_indexs_arrays(abc, subNode, 0);
					update_slp_child_config(abc, subNode, 0);
				}
			} else if(!xmlStrcmp(szPropity, (const xmlChar *) "low power")) {
				if(abc->mode == 2) {
					update_slp_config_indexs_arrays(abc, subNode, 1);
					update_slp_child_config(abc, subNode, 1);
				}
			}
		}
		subNode = subNode->next;
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
		if (!xmlStrcmp(curNode->name, (const xmlChar*)"epf_config")) {
			ENG_LOG("aabbcc %s \n", curNode->name);
			parse_epf_config(abc, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"slp_config")) {
			ENG_LOG("aabbcc %s \n", curNode->name);
			parse_slp_config(abc, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"slp_mapping_table")) {
			ENG_LOG("aabbcc %s \n", curNode->name);
			parse_slp_mapping_table(abc, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"bl_mapping_table")) {
			ENG_LOG("aabbcc %s \n", curNode->name);
			parse_bl_mapping_table(abc, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"enhance")) {
			ENG_LOG("aabbcc %s \n", curNode->name);
			parse_abc_version(abc, curNode);
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
		if (!xmlStrcmp(curNode->name, (const xmlChar*)"epf_config")) {
			ENG_LOG("aabbcc0 %s \n", curNode->name);
			update_epf_config(abc, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"slp_config")) {
			ENG_LOG("aabbcc0 %s \n", curNode->name);
			update_slp_config(abc, curNode);
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"slp_mapping_table")) {
			ENG_LOG(" PQ curNode name %s \n",curNode->name);
			if(abc->slpblMode == 1) {
				ENG_LOG("aabbcc0 %s \n", curNode->name);
				update_slp_mapping_table(abc, curNode);
		}
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"bl_mapping_table")) {
			ENG_LOG("PQ 1curNode name %s \n",curNode->name);
			if(abc->slpblMode == 2) {
				ENG_LOG("aabbcc0 %s \n", curNode->name);
				update_bl_mapping_table(abc, curNode);
		}
		} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"enhance")) {
			ENG_LOG("aabbcc0 %s \n", curNode->name);
			update_abc_version(abc, curNode);
		}
		curNode = curNode->next;
	}
	xmlSaveFormatFileEnc(abc_xml, doc, "UTF-8", 1);
	xmlFreeDoc(doc);

	return 0;
}
