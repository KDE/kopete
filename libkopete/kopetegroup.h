/*
    kopetegroup.h - Kopete (Meta)Contact Group

    Copyright (c) 2002-2005 by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

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

#include <QtCore/QList>

#include "kopetecontactlistelement.h"

#include "kopete_export.h"


namespace Kopete {


class MetaContact;
class Message;

/**
 * Class which represents the Group.
 *
 * A Group is a ConstacListElement which means plugin can save data.
 *
 * some static group are availavle from this class:  topLevel and temporary
 *
 * @author Olivier Goffart <ogoffart@kde.org>
 */
class KOPETE_EXPORT Group : public ContactListElement
{
	Q_PROPERTY( QString displayName READ displayName WRITE setDisplayName )
	Q_PROPERTY( uint groupId READ groupId )
	Q_PROPERTY( bool expanded READ isExpanded WRITE setExpanded )

	Q_OBJECT

public:
	typedef QList<Group*> List;

	/** Kinds of groups. */
	enum GroupType { Normal=0, Temporary, TopLevel, Offline };

	/**
	 * \brief Create an empty group
	 *
	 * Note that the constructor will not add the group automatically to the contact list.
	 * Use @ref ContactList::addGroup() to add it
	 */
	Group();

	/**
	 * \brief Create a group of the specified type
	 *
	 * Overloaded constructor to create a group with a display name and Normal type.
	 */
	explicit Group( const QString &displayName );

	~Group();

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
	 * \return the unique id for this group
	 */
	uint groupId() const;

	/**
	 * @brief child metacontact
	 * This function is not very efficient - it searches through all the metacontacts in the contact list
	 * \return the members of this group
	 */
	QList<MetaContact *> members() const;

	/**
	 * \brief Set if the group is expanded.
	 */
	void setExpanded( bool expanded );

	/**
	 *
	 * \return true if the group is expanded.
	 * \return false otherwise
	 */
	bool isExpanded() const;

	/**
	 * \return a Group pointer to the toplevel group
	 */
	static Group *topLevel();

	/**
	 * \return a Group pointer to the temporary group
	 */
	static Group *temporary();

	/**
	 * \return a Group pointer to the offline group
	 */
	static Group *offline();

	/**
	 * @internal
	 */
	void setGroupId( uint groupId );

	/**
	 * @internal
	 */
	uint uniqueGroupId() const;
	/**
	 * @internal
	 */
	void setUniqueGroupId( uint uniqueGroupId );

public slots:
	/**
	 * Send a message to all contacts in the group
	 */
	void sendMessage();


signals:
	/**
	 * \brief Emitted when the group has been renamed
	 */
	void displayNameChanged( Kopete::Group *group , const QString &oldName );

	
private slots:
	void sendMessage( Kopete::Message& );

private:
	/**
	 * \brief Create a group of the specified type
	 */
	Group( const QString &displayName, GroupType type );

	static Group *s_topLevel;
	static Group *s_temporary;
	static Group *s_offline;

	class Private;
	Private * const d;

	/**
	 * @internal  used to get reachabe contact to send message to thom.
	 */
	QList<MetaContact *> onlineMembers() const;
};

} //END namespace Kopete 

#endif


