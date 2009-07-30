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

#include "kopetecontact.h"
#include "kopeteonlinestatus.h"
#include "chatsessionmemberslistmodel.h"

namespace Kopete
{

ChatSessionMembersListModel::ChatSessionMembersListModel(QObject * parent)
	: QAbstractListModel(parent), m_session(0L)
{

}

void ChatSessionMembersListModel::setChatSession(ChatSession *session)
{
	if ( m_session )
		disconnect( m_session, 0, this, 0 );

	m_session = session;

	connect( session, SIGNAL(closing(Kopete::ChatSession*)),
	         this, SLOT(slotSessionClosed()) );
	connect( session, SIGNAL(contactAdded(const Kopete::Contact*, bool)),
	         this, SLOT(slotContactAdded(const Kopete::Contact*)) );
	connect( session, SIGNAL(contactRemoved(const Kopete::Contact*, const QString&, Qt::TextFormat, bool)),
	         this, SLOT(slotContactRemoved(const Kopete::Contact*)) );
	connect( session, SIGNAL(onlineStatusChanged(Kopete::Contact*, const Kopete::OnlineStatus&, const Kopete::OnlineStatus&)),
	         this, SLOT(slotContactStatusChanged(Kopete::Contact*, const Kopete::OnlineStatus&)) );
	connect( session, SIGNAL(displayNameChanged()),
	         this, SLOT(slotSessionChanged()) );
	connect( session, SIGNAL(photoChanged()),
	         this, SLOT(slotSessionChanged()) );
	reset();
}

Kopete::Contact * ChatSessionMembersListModel::contactAt( const QModelIndex &index ) const
{
	if ( m_session )
	{
		if (!index.isValid())
			return 0L;
	
		if (index.row() >= m_session->members().size() + 1)
			return 0L;

		if ( index.row() == 0 )
			return const_cast<Kopete::Contact *>(m_session->myself());
		else
			return m_session->members().at(index.row() - 1);
	}

	return 0L;
}

int ChatSessionMembersListModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	if ( m_session )
		return m_session->members().count() + 1;
	
	return 0;
}

QVariant ChatSessionMembersListModel::data(const QModelIndex &index, int role) const
{
	Contact *c = contactAt(index);
	if (!c)
		return QVariant();

	if (role == Qt::DisplayRole)
	{
		QString nick = c->property(Kopete::Global::Properties::self()->nickName().key()).value().toString();
		if ( nick.isEmpty() )
			nick = c->contactId();
		
		return nick;
	}
	else if (role == Qt::DecorationRole)
	{
		return m_session->contactOnlineStatus(c).iconFor(c);
	}
	else if (role == Qt::ToolTipRole)
	{
		return c->toolTip();
	}
	else
		return QVariant();
}

QVariant ChatSessionMembersListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
		return QString("Column %1").arg(section);
	else
		return QString("Row %1").arg(section);
}

void ChatSessionMembersListModel::slotContactAdded( const Kopete::Contact *contact )
{
	Q_UNUSED(contact)
	// NOTE in the future the adding of a contact
  // could be done just for the contact
	reset();
}

void ChatSessionMembersListModel::slotContactRemoved( const Kopete::Contact *contact )
{
	Q_UNUSED(contact)
	// NOTE in the future the removal of a contact
  // could be done just for the contact
	reset();
}

void ChatSessionMembersListModel::slotContactStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &status )
{
	Q_UNUSED(contact)
	Q_UNUSED(status)
	// NOTE in the future the change of a contact
  // could be done just for the contact
	reset();
}

void ChatSessionMembersListModel::slotSessionChanged()
{
	reset();
}

void ChatSessionMembersListModel::slotSessionClosed()
{
	if ( m_session )
	{
		disconnect( m_session, 0, this, 0 );
		m_session = 0;
		reset();
	}
}

}
