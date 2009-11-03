/*
    ChatSessionMembersListModel

    Copyright (c) 2007 by Duncan Mac-Vicar Prett <duncan@kde.org>
   
    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef Kopete_ChatSessionMembersListModel_H
#define Kopete_ChatSessionMembersListModel_H

#include <QAbstractListModel>

#include "kopetechatsession.h"
#include "kopete_export.h"

class Private;

namespace Kopete
{

class Contact;

class KOPETE_EXPORT ChatSessionMembersListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit ChatSessionMembersListModel(QObject * parent = 0);

	~ChatSessionMembersListModel();

	// Model methods
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	Kopete::ChatSession *session();

	Kopete::Contact *contactAt( const QModelIndex &index ) const;
public slots:
	/**
	 * Called when the ChatSession change for this list (eg. when the tab in the KopeteChatWindow is changing)
	 */
	void setChatSession(Kopete::ChatSession *session);

private slots:
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

	 /** Called when the nickname of a contact changed.
	 * @param contact The contact who changed nickname
	 */
	void slotContactNickNameChanged( Kopete::Contact *contact);

	/**
	 * Called when a contact changes status.
	 * @param contact The contact who changed status
	 * @param status The new status of the contact
	 */
	void slotContactStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &status );

	/**
	 * Called when something in the session changed that requires a full
	 * model reset
	 */
	void slotSessionChanged();

	/**
	 * Called when session has been closed
	 */
	void slotSessionClosed();

private:
	Private *d;
};


}

#endif

