/*
    kopetemetacontactlvi.h - Kopete Meta Contact KListViewItem

    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002      by Duncan Mac-Vicar P     <duncan@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __kopetemetacontactlvi_h__
#define __kopetemetacontactlvi_h__

#include <qobject.h>
#include <qpixmap.h>
#include <qptrdict.h>

#include <klistview.h>


class KAction;
class KListAction;
class KopeteAccount;
class KopeteMetaContact;

class AddContactPage;
class KopeteContact;
class KopeteGroupViewItem;
class KopeteGroup;
class KopeteEvent;

/**
 * @author Martijn Klingens <klingens@kde.org>
 */
class KopeteMetaContactLVI : public QObject, public KListViewItem
{
	Q_OBJECT

public:
	KopeteMetaContactLVI( KopeteMetaContact *contact, KopeteGroupViewItem *parent );
	KopeteMetaContactLVI( KopeteMetaContact *contact, QListViewItem *parent );
	KopeteMetaContactLVI( KopeteMetaContact *contact, QListView *parent );
	/**
	 * Copy constructor.
	 * Makes a second view of the same meta contact, if the meta contact
	 * resides in multiple groups.
	 */
//	KopeteMetaContactLVI( KopeteMetaContactLVI *other, QListViewItem *parent );
	~KopeteMetaContactLVI();

	/**
	 * metacontact this visual item represents
	 */
	KopeteMetaContact *metaContact()
	{ return m_metaContact; };

	/**
	 * true if the item is at top level and not under a group
	 */
	bool isTopLevel();

	/**
	 * parent when top-level
	 */
	QListView *parentView()
	{ return m_parentView; };

	/**
	 * parent when not top-level
	 */
	KopeteGroupViewItem *parentGroup()
	{ return m_parentGroup; };

//	virtual void setup();
	virtual void paintCell ( QPainter *p, const QColorGroup &cg, int column, int width, int align );

	/* Duncan experiment */
/*	virtual void setColor( const QColor &color );
	virtual void setColor( const QString &color );
	virtual void setText( int column, const QString &text );
	QColor color();
	QString colorName();*/

	void showContextMenu( const QPoint &point );

	/**
	 * Call the meta contact's execute as I don't want to expose m_contact
	 * directly.
	 */
	void execute() const;

	void movedToGroup(KopeteGroup * );
	void rename( const QString& name );

	KopeteGroup *group();

	/**
	 *  Return the KopeteContact of the small little icon at he point p
	 *  p must be coord of the cell
	 *  Return 0L if p is not on a smallicon
	 *  (This is used to showing context-menu of a contact when right-click on a icon)
	 */
	KopeteContact *getContactFromIcon(const QPoint &p);

	bool isGrouped();

	void catchEvent(KopeteEvent *);

private slots:
	void slotUpdateIcons();
	void slotContactStatusChanged();
	
	void slotDisplayNameChanged();
	void slotRemoveThisUser();
	void slotRemoveFromGroup();
	void slotMoveToGroup();
	void slotAddContact();
	void slotAddTemoraryContact();

	void slotAddToGroup();
	void slotAddToNewGroup();
	void slotRename();
	void slotIdleStateChanged();

	/**
	 * maybe remove this and find a better way to check for pref-changes
	 */
	void slotConfigChanged();

	void slotEventDone(KopeteEvent* );
	void slotBlink();

protected:
	virtual void okRename(int col);

private:
	void initLVI();
	QString key( int column, bool ascending ) const;

	void updateVisibility();

	KopeteMetaContact *m_metaContact;

	// Used actions in the context menu
	KListAction *m_actionMove;
	KListAction *m_actionCopy;

	KopeteGroupViewItem *m_parentGroup;
	QListView *m_parentView;
	bool m_isTopLevel;

	int m_pixelWide;

	KopeteEvent *m_event;
	QTimer *mBlinkTimer;
	QPixmap mBlinkIcon;
	bool mIsBlinkIcon;

	QPtrDict<KopeteAccount> m_addContactActions;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

