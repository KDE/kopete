/*
    chatmemberslistwidget.h - Chat Members List Widget

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef CHATMEMBERSLISTWIDGET_H
#define CHATMEMBERSLISTWIDGET_H

#include <klistview.h>

#include <qmap.h>

namespace Kopete
{
class ChatSession;
class Contact;
class OnlineStatus;
}

/**
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class ChatMembersListWidget : public KListView
{
	Q_OBJECT
public:
	ChatMembersListWidget( Kopete::ChatSession *session, QWidget *parent, const char *name = 0 );
	virtual ~ChatMembersListWidget();

	Kopete::ChatSession *session() { return m_session; }

	class ToolTip;
	class ContactItem;

protected:

	/**
	 * Start a drag operation
	 * @return a KMultipleDrag containing:
	 *	1) A QStoredDrag of type "application/x-qlistviewitem",
	 *	2) If the contact is associated with a KABC entry,
	 *		i) a QTextDrag containing their email address, and
	 *		ii) their vCard representation.
	 */
	virtual QDragObject *dragObject();

private slots:
	/**
	 * Show the context menu for @p item at @p point
	 */
	void slotContextMenu( KListView*, QListViewItem *item, const QPoint &point );

	/**
	 * Called when a contact is added to the chat session.
	 * Adds this contact to the contact list view.
	 * @param c The contact that joined the chat
	 */
	void slotContactAdded( const Kopete::Contact *c );

	/**
	 * Called when a contact is removed from the chat session.
	 * Removes this contact from the contact list view.
	 * @param c The contact that left the chat
	 */
	void slotContactRemoved( const Kopete::Contact *c );

	/**
	 * Called when a contact changes status.
	 * @param contact The contact who changed status
	 * @param status The new status of the contact
	 */
	void slotContactStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &status );

	/**
	 * Called when a contact is clicked.
	 * @param item The list view item representing the clicked contact
	 */
	void slotExecute( QListViewItem *contact );

private:
	Kopete::ChatSession *m_session;
	QMap<const Kopete::Contact*, ContactItem*> m_members;
	ToolTip *m_toolTip;
};

class ChatMembersListWidget::ContactItem : public QObject, public KListViewItem
{
	Q_OBJECT
public:
	ContactItem( ChatMembersListWidget *list, Kopete::Contact *contact );
	Kopete::Contact *contact() const { return m_contact; }

private slots:
	void slotPropertyChanged( Kopete::Contact *contact, const QString &key, const QVariant &oldValue, const QVariant &newValue  );

private:
	friend class ChatMembersListWidget;

	void reposition();
	void setStatus( const Kopete::OnlineStatus &status );

	Kopete::Contact *m_contact;
};


#endif

// vim: set noet ts=4 sts=4 sw=4:

