/***************************************************************************
                          kopetegroup.h  -  description
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

#ifndef KOPETEGROUP_H
#define KOPETEGROUP_H

#include <qobject.h>
#include <qstringlist.h>
#include <qvaluelist.h>

/**
  *@author Olivier Goffart
  */

//class KopeteGroup;

class KopeteGroup : public QObject 
{
public:
	enum GroupType { Classic, Temporary, TopLevel};

	KopeteGroup();
	KopeteGroup(QString name, GroupType type=Classic);
//	KopeteGroup(const KopeteGroup &);
	~KopeteGroup();

	QString displayName() const ;
	void setDisplayName (const QString&);

	GroupType type() const ;
	void setType(GroupType);

	static KopeteGroup *toplevel;
	static KopeteGroup *temporary;

private:
	QString m_displayName;
	GroupType m_type;
	
};

class KopeteGroupList : public  QPtrList<KopeteGroup>
{
	public:
		KopeteGroupList();
		~KopeteGroupList();
		QStringList toStringList();
};

/*bool operator ==(const KopeteGroup &, const KopeteGroup &);
bool operator !=(const KopeteGroup &, const KopeteGroup &);*/


#endif
