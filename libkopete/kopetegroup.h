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

#include "kopeteplugindataobject.h"
#include <qstringlist.h>
#include <qvaluelist.h>

class QDomElement;
class KopetePlugin;
struct KopeteGroupPrivate;

/**
 * @author Olivier Goffart
 */
class KopeteGroup : public KopetePluginDataObject
{
	Q_OBJECT

public:
	enum GroupType { Classic, Temporary, TopLevel};

	KopeteGroup();
	KopeteGroup(QString name, GroupType type=Classic);
//	KopeteGroup(const KopeteGroup &);
	~KopeteGroup();

	/**
	 *   return the group displayName
	 */
	QString displayName() const ;

	/*
	 *  rename the group
	 */
	void setDisplayName (const QString&);

	GroupType type() const ;
	void setType(GroupType);

	QString toXML();
	bool fromXML( const QDomElement &data );

	void setExpanded(bool in_expanded) ;
	bool expanded() ;

	static KopeteGroup *toplevel;
	static KopeteGroup *temporary;

private:
	KopeteGroupPrivate *d;
	


signals: 
  void renamed(KopeteGroup* , const QString& );
};

//FIXME: should be typedef QPtrList<KopeteGroup> KopeteGroupList
class KopeteGroupList : public  QPtrList<KopeteGroup>
{
	public:
		KopeteGroupList();
		~KopeteGroupList();
		QStringList toStringList();
};


#endif
