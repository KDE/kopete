/*
    kopetegroup.h - Kopete's Group

    Copyright (c) 2002 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

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
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qdom.h>

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
	/**
	 * Constructor: note that the constructor doesn't add the group automatically to the contactlist.
	 * use @ref KopeteContactList::addGroup() to add it
	 */
	KopeteGroup(QString name, GroupType type=Classic);
//	KopeteGroup(const KopeteGroup &);
	~KopeteGroup();

	/**
	 *   return the group displayName
	 */
	QString displayName() const ;

	/**
	 *  rename the group
	 */
	void setDisplayName (const QString&);

	/**
	 * return the group type
	 */
	GroupType type() const ;
	/**
	 * set the group type
	 */
	void setType(GroupType);

	/**
	 * Return the unique id for this group
	 */
	unsigned int groupId() const;

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
	void setExpanded(bool in_expanded) ;
	/**
	 * say if the group is expanded or not,
	 */
	bool expanded() const;

	/**
	 * set the parent group
	 * a group = 0L means the top-level group
	 */
	//void setParentGroup(KopeteGroup*);
	/**
	 * Accessor to the parent group.
	 */
	//KopeteGroup* parentGroup();


	/**
	 * a link to the toplevel group
	 */
	static KopeteGroup *toplevel;
	/**
	 * a link to the temporary group
	 */
	static KopeteGroup *temporary;

signals:
	/**
	 * The group has been renamed
	 */
	void renamed(KopeteGroup* , const QString& );
	/**
	 * The group has changed parents
	 */
	//void movedToGroup( KopeteGroup *from , KopeteGroup *to, KopeteGroup *this_one );

private:
	KopeteGroupPrivate *d;
};

typedef QPtrList<KopeteGroup> KopeteGroupList;

#endif
