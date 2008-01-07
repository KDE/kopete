#include "util_Xml.h"
#include "malloc.h"

char * util_Xml_nodeValue(IXML_Node* nodeptr)
{
	int sizeBalise = strlen(ixmlNode_getNodeName(nodeptr));
	int sizeChaine = strlen(ixmlNodetoString(nodeptr));
	char * chaine = ixmlNodetoString(nodeptr);
	int deb = sizeBalise +2;
	int fin = sizeChaine-(sizeBalise+3);
	int tailleMax = fin - deb +1;

	char * ret= (char *)malloc(tailleMax);
	
	strncpy(ret,&chaine[deb],tailleMax-1);
	ret[tailleMax-1] = '\0';
	
	return ret;
}


