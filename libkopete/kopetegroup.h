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
class KopeteMetaContact;

/**
 * @author Olivier Goffart
 */
class KopeteGroup : public KopetePluginDataObject
{
	Q_PROPERTY( QString displayName READ displayName WRITE setDisplayName )
	Q_PROPERTY( uint groupId READ groupId )
	Q_PROPERTY( bool expanded READ isExpanded WRITE setExpanded )

	Q_OBJECT

public:
	enum GroupType { Normal=0, Temporary, TopLevel };

	/**
	 * \brief Create an empty group
	 *
	 * Note that the constructor will not add the group automatically to the contact list.
	 * Use @ref KopeteContactList::addGroup() to add it
	 */
	KopeteGroup();

	/**
	 * \brief Create a group of the specified type
	 *
	 * Overloaded constructor to create a group of the specified type.
	 */
	KopeteGroup( const QString &name, GroupType type = Normal );

	~KopeteGroup();

	/**
	 * \brief Return the group's display name
	 *
	 * \return the display name of the group
	 */
	QString displayName() const;

	/**
	 * \brief Rename the group
	 */
	void setDisplayName( const QString &newName );

	/**
	 * \return the group type
	 */
	GroupType type() const;

	/**
	 * \brief Set the group type
	 */
	void setType( GroupType newType );

	/**
	 * \return the unique id for this group
	 */
	uint groupId() const;

	/**
	 * \return the members of this group
	 */
	QPtrList<KopeteMetaContact> members() const;

	/**
	 * @internal
	 * Outputs the group data in XML
	 */
	const QDomElement toXML();

	/**
	 * @internal
	 * Loads the group data from XML
	 */
	bool fromXML( const QDomElement &data );

	/**
	 * \brief Set if the group is expanded.
	 *
	 * This is saved to the xml contactlist file
	 * FIXME: the group should not need to know this
	 */
	void setExpanded( bool expanded );

	/**
	 *
	 * \return true if the group is expanded.
	 * \return false otherwise
	 */
	bool isExpanded() const;

	/**
	 * \return a KopeteGroup pointer to the toplevel group
	 */
	static KopeteGroup *topLevel();

	/**
	 * \return a KopeteGroup pointer to the temporary group
	 */
	static KopeteGroup *temporary();

signals:
	/**
	 * \brief Emitted when the group has been renamed
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

