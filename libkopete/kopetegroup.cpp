/***************************************************************************
                          kopetegroup.cpp  -  description
                             -------------------
    begin                : jeu oct 24 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kopetegroup.h"
#include "kopetecontactlist.h"

KopeteGroup* KopeteGroup::toplevel=new KopeteGroup(QString::null , KopeteGroup::TopLevel);
KopeteGroup* KopeteGroup::temporary=new KopeteGroup("temporaryGroup",KopeteGroup::Temporary);

/*bool operator==(const KopeteGroup &a, const KopeteGroup &b)
{
	if(a.type()==KopeteGroup::TopLevel && b.type()==KopeteGroup::TopLevel)
		return true;
	if(a.type() != b.type()) return false;
	if(a.displayName() != b.displayName()) return false;
	return true;
}

bool operator!=(const KopeteGroup &a, const KopeteGroup &b)
{
	return !(a==b);
}*/


KopeteGroupList::KopeteGroupList(){}
KopeteGroupList::~KopeteGroupList(){}

QStringList KopeteGroupList::toStringList() 
{
	QStringList rep;
	KopeteGroup *g;;
	for ( g = first(); g; g = next() )
	{
		//if((*it).type() == KopeteGroup::Classic && !(*it).string().isNull())
		rep << g->displayName();
	}
	return rep;
}

KopeteGroup::KopeteGroup(QString _name, GroupType _type)  : QObject(KopeteContactList::contactList())
{
	m_displayName=_name;
	m_type=_type;
}
KopeteGroup::KopeteGroup()  : QObject(KopeteContactList::contactList())
{
	m_type=Classic;
	m_displayName=QString::null;
}
/*KopeteGroup::KopeteGroup(const KopeteGroup &a)
{
	m_displayName=a.displayName();
	m_type=a.type();
} */

KopeteGroup::~KopeteGroup()
{
}

void KopeteGroup::setDisplayName(const QString &s)
{
	m_displayName=s;
}
QString KopeteGroup::displayName() const
{
	return m_displayName;
}


KopeteGroup::GroupType KopeteGroup::type() const
{
	return m_type;
}
void KopeteGroup::setType(GroupType t)
{
	m_type=t;
}


