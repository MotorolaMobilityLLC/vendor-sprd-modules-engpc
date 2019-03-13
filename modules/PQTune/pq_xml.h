#include <libxml/parser.h>
#include <libxml/tree.h>
#include <../libxml.h>

#define bld_xml             "/mnt/vendor/enhance/bld.xml"
#define abc_xml             "/mnt/vendor/enhance/abc.xml"
#define gamma_xml           "/mnt/vendor/enhance/gamma.xml"
#define cms_xml             "/mnt/vendor/enhance/cms.xml"



extern xmlNodePtr FindNode(xmlNodePtr curNode, const char *name);
extern void RemoveNode(xmlNode *ParentNode, xmlNode *ChildNode);
