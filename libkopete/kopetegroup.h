/*
    kopetegroup.h - Kopete (Meta)Contact Group

    Copyright (c) 2002-2003 by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEGROUP_H
#define KOPETEGROUP_H

#include "kopeteplugindataobject.h"

#include <qptrlist.h>

class QDomElement;

class KopeteGroupPrivate;

/**
 * @author Olivier Goffart
 */
class KopeteGroup : public KopetePluginDataObject
{
	Q_OBJECT

public:
	enum GroupType { Normal, Temporary, TopLevel };

	/**
	 * Create an empty group
	 * Note that the constructor doesn't add the group automatically to the contactlist.
	 * use @ref KopeteContactList::addGroup() to add it
	 */
	KopeteGroup();

	/**
	 * Overloaded constructor to create a group of the specified type.
	 */
	KopeteGroup( const QString &name, GroupType type = Normal );

	~KopeteGroup();

	/**
	 * return the group displayName
	 */
	QString displayName() const;

	/**
	 *  rename the group
	 */
	void setDisplayName( const QString &newName );

	/**
	 * return the group type
	 */
	GroupType type() const;

	/**
	 * set the group type
	 */
	void setType( GroupType newType );

	/**
	 * Return the unique id for this group
	 */
	uint groupId() const;

	/**
	 * @internal
	 */
	const QDomElement toXML();

	/**
	 * @internal
	 */
	bool fromXML( const QDomElement &data );

	/**
	 * set if the group is expanded. this is saved to the xml contactlist file
	 */
	void setExpanded( bool expanded );

	/**
	 * return whether the group is expanded or not,
	 */
	bool isExpanded() const;

	/**
	 * a link to the toplevel group
	 */
	static KopeteGroup *topLevel();

	/**
	 * a link to the temporary group
	 */
	static KopeteGroup *temporary();

signals:
	/**
	 * The group has been renamed
	 */
	void renamed( KopeteGroup *group , const QString &oldName );

private:
	static KopeteGroup *s_topLevel;
	static KopeteGroup *s_temporary;

	KopeteGroupPrivate *d;
};

typedef QPtrList<KopeteGroup> KopeteGroupList;

#endif

// vim: set noet ts=4 sts=4 sw=4:

