/*
    Model view item class for Kopete::Group

    Copyright (c) 2002 by Olivier Goffart <ogoffart@kde.org>
    Copyright (c) 2007 by Matt Rogers     <mattr@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#ifndef KOPETEGROUPVIEWITEM_H
#define KOPETEGROUPVIEWITEM_H

#include <QtGui/QStandardItem>

#define KOPETE_GROUP_DEFAULT_OPEN_ICON "folder-open"
#define KOPETE_GROUP_DEFAULT_CLOSED_ICON "folder"

namespace Kopete
{
class Group;
}

/**
 * @author Olivier Goffart
 */
class KopeteGroupViewItem : public QObject, QStandardItem 
{
	Q_OBJECT
public:
	explicit KopeteGroupViewItem( Kopete::Group *group );
	~KopeteGroupViewItem();

	void setGroup( Kopete::Group* group );
	Kopete::Group * group() const;

public Q_SLOTS:

	void groupNameChanged( Kopete::Group*, const QString& );
private:
	Kopete::Group* m_group;
};

#endif
