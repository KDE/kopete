/***************************************************************************
                          kopetegroupviewitem.h  -  description
                             -------------------
    begin                : lun oct 28 2002
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

#ifndef KOPETEGROUPVIEWITEM_H
#define KOPETEGROUPVIEWITEM_H

#include "kopetelistviewitem.h"
#include <qpixmap.h>

/**
  *@author Olivier Goffart
  */

class KopeteGroup;

class KopeteGroupViewItem : public Kopete::UI::ListView::Item
{
	Q_OBJECT
public:
	KopeteGroupViewItem( KopeteGroup *group , QListView *parent, const char *name = 0 );
	KopeteGroupViewItem( KopeteGroup *group , QListViewItem *parent, const char *name = 0 );
	~KopeteGroupViewItem();

	KopeteGroup * group() const;

	virtual void startRename( int col );

protected:
	virtual void okRename( int col );
	virtual void cancelRename( int col );

private:
	void initLVI();

	KopeteGroup *m_group;
	QPixmap open, closed;

	QString key( int column, bool ascending ) const;

	QString m_renameText;

	unsigned int onlineMemberCount;
	unsigned int totalMemberCount;

	class Private;
	Private *d;

public slots:
	void refreshDisplayName();
	void updateIcon();
	void updateVisibility();
};

#endif
