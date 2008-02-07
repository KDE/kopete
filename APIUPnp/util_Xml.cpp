/*
    util_Xml.cpp -

    Copyright (c) 2007-2008 by Romain Castan      <romaincastan@gmail.com>
    Copyright (c) 2007-2008 by Bertrand Demay     <bertranddemay@gmail.com>
    Copyright (c) 2007-2008 by Julien Hubatzeck   <reineur31@gmail.com>
    Copyright (c) 2007-2008 by Michel Saliba      <msalibaba@gmail.com>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

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


