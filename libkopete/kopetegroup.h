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

class QDomNode;
class KopetePlugin;

class KopeteGroup : public QObject 
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
	bool fromXML( const QDomNode &data );
	
	void setExpanded(bool in_expanded) { m_expanded = in_expanded; }
	bool expanded() { return m_expanded; }

	/**
	 * Set or get data specific for each plugin.
	 * Theses data are saved to the contactlist
	 */
	void setPluginData(KopetePlugin *p, QStringList value );
	QStringList pluginData(KopetePlugin *p) ;


	static KopeteGroup *toplevel;
	static KopeteGroup *temporary;

private:
	QString m_displayName;
	GroupType m_type;
	bool m_expanded;
  	QMap<QString, QString> m_pluginData;

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
