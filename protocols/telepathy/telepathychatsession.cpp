/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "telepathychatsession.h"
#include "telepathyprotocol.h"
#include "common.h"

#include <kopetechatsessionmanager.h>

#include <kdebug.h>

#include <TelepathyQt4/Client/Contact>
#include <TelepathyQt4/Client/Connection>
#include <TelepathyQt4/Client/ContactManager>
#include <TelepathyQt4/Client/PendingChannel>

TelepathyChatSession::TelepathyChatSession(const Kopete::Contact *user, Kopete::ContactPtrList others, Kopete::Protocol *protocol)
	: Kopete::ChatSession(user, others, protocol)
{
	kDebug(TELEPATHY_DEBUG_AREA);
	Kopete::ChatSessionManager::self()->registerChatSession(this);
	
	QObject::connect(this, SIGNAL(messageSent(Kopete::Message&, Kopete::ChatSession*)), this, SLOT(sendMessage(Kopete::Message&)));
	QObject::connect(this, SIGNAL(closing(Kopete::ChatSession *)), this, SLOT(closingChatSession(Kopete::ChatSession *)));
}

TelepathyChatSession::~TelepathyChatSession()
{
	kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyChatSession::createTextChannel(QSharedPointer<Telepathy::Client::Contact> contact)
{
	kDebug(TELEPATHY_DEBUG_AREA);
	m_contact = contact;
	
	Telepathy::Client::Connection *connection = contact->manager()->connection();
	
    QVariantMap request;
    request.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
                   TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT);
    request.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
                   Telepathy::HandleTypeContact);
    request.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandle"),
                   connection->selfHandle());
	
	QObject::connect(connection->createChannel(request),
		SIGNAL(finished(Telepathy::Client::PendingOperation*)),
		this,
		SLOT(createChannelFinished(Telepathy::Client::PendingOperation*)));
}

void TelepathyChatSession::sendMessage(Kopete::Message &message)
{
	kDebug(TELEPATHY_DEBUG_AREA);
	
	if(!m_textChannel)
		return;
	
	m_textChannel->send(message.parsedBody());
}

void TelepathyChatSession::createChannelFinished(Telepathy::Client::PendingOperation* operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);
    
    if(TelepathyCommons::isOperationError(operation))
        return;
	
	Telepathy::Client::PendingChannel *pc = qobject_cast<Telepathy::Client::PendingChannel*>(operation);
	
	if(!pc)
	{
		kDebug(TELEPATHY_DEBUG_AREA) << "Error PendingChannel casting!";
		return;
	}

	m_textChannel = pc->channel();
}

void TelepathyChatSession::closingChatSession(Kopete::ChatSession *kmm)
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	Q_UNUSED(kmm);
	
	if(!m_textChannel)
		return;
	
	QObject::connect(m_textChannel->requestClose(),
		SIGNAL(finished(Telepathy::Client::PendingOperation*)),
		this,
		SLOT(chatSessionRequestClose(Telepathy::Client::PendingOperation*)));
}

void TelepathyChatSession::chatSessionRequestClose(Telepathy::Client::PendingOperation *operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);
    
    if(TelepathyCommons::isOperationError(operation))
        return;
	
    kDebug(TELEPATHY_DEBUG_AREA) << "Chat session closed";
}



















