/*
    kopetegroupviewitem.h  -  description
                             -------------------
    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

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

#include "kopetelistviewitem.h"
#include <qpixmap.h>

#define KOPETE_GROUP_DEFAULT_OPEN_ICON "folder-open"
#define KOPETE_GROUP_DEFAULT_CLOSED_ICON "folder"

namespace Kopete
{
class Group;
}

/**
 * @author Olivier Goffart
 */
class KopeteGroupViewItem : public Kopete::UI::ListView::Item
{
	Q_OBJECT
public:
	KopeteGroupViewItem( Kopete::Group *group , Q3ListView *parent );
	KopeteGroupViewItem( Kopete::Group *group , Q3ListViewItem *parent );
	~KopeteGroupViewItem();

	Kopete::Group * group() const;

	virtual void startRename( int col );

	/**
	 * reimplemented from K3ListViewItem to take into account our alternate text storage
	 */
	virtual QString text( int column ) const;
	virtual void setText( int column, const QString &text );

	QString toolTip() const;

public slots:
	void refreshDisplayName();
	void updateIcon();
	void updateVisibility();

protected:
	virtual void okRename( int col );
	virtual void cancelRename( int col );

private:
	void initLVI();

	Kopete::Group *m_group;
	QPixmap open, closed;

	QString key( int column, bool ascending ) const;

	unsigned int onlineMemberCount;
	unsigned int totalMemberCount;

	class Private;
	Private * const d;

private slots:
	void slotConfigChanged();
};

#endif
