/*
    kopetegroup_p.h - Kopete (Meta)Contact Group Private

    Copyright (c) 2002-2005 by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEGROUP_P_H
#define KOPETEGROUP_P_H

#include "kopetegroup.h"

namespace Kopete {

class Group::Private
{
public:
	QString displayName;
	Group::GroupType type;
	bool expanded;
	uint groupId;

	//Unique contact id per metacontact
	static uint uniqueGroupId;
};

} //END namespace Kopete 

#endif
