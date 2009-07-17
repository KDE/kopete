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

#include "telepathyaccount.h"
#include "telepathyprotocol.h"
#include "common.h"

#include <kopetechatsessionmanager.h>

#include <kdebug.h>

#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingChannelRequest>
#include <TelepathyQt4/PendingReady>

TelepathyChatSession::TelepathyChatSession(const Kopete::Contact *user, Kopete::ContactPtrList others, Kopete::Protocol *protocol)
        : Kopete::ChatSession(user, others, protocol),
        m_pendingChannelRequest(0)
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

void TelepathyChatSession::createTextChannel(QSharedPointer<Tp::Contact> contact)
{
    kDebug(TELEPATHY_DEBUG_AREA);
    m_contact = contact;

    QString preferredHandler("org.freedesktop.Telepathy.Client.KopetePlugin");

    TelepathyAccount *telepathyAccount = qobject_cast<TelepathyAccount*>(account());
    if (!telepathyAccount) {
        kWarning(TELEPATHY_DEBUG_AREA) << "Null telepathy account. Fail.";
        return;
    }

    Tp::AccountPtr account = telepathyAccount->account();
    if (account.isNull()) {
        kWarning(TELEPATHY_DEBUG_AREA) << "Null account. Fail.";
        return;
    }

    m_pendingChannelRequest =
            account->ensureTextChat(contact, QDateTime::currentDateTime(), preferredHandler);

    m_channelRequest = m_pendingChannelRequest->channelRequest();
    kDebug(TELEPATHY_DEBUG_AREA) << "m_channelRequest:" << m_channelRequest.data();

    connect(m_pendingChannelRequest,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onEnsureChannelFinished(Tp::PendingOperation*)));
}

void TelepathyChatSession::sendMessage(Kopete::Message &message)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if (!m_textChannel)
        return;

    m_textChannel->send(message.plainBody());
}

void TelepathyChatSession::onEnsureChannelFinished(Tp::PendingOperation* operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if (TelepathyCommons::isOperationError(operation))
        return;

    m_channelRequest = m_pendingChannelRequest->channelRequest();
    m_pendingChannelRequest = 0;
}

void TelepathyChatSession::closingChatSession(Kopete::ChatSession *kmm)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    Q_UNUSED(kmm);

    if (!m_textChannel)
        return;

    QObject::connect(m_textChannel->requestClose(),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     this,
                     SLOT(chatSessionRequestClose(Tp::PendingOperation*)));
}

void TelepathyChatSession::chatSessionRequestClose(Tp::PendingOperation *operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if (TelepathyCommons::isOperationError(operation))
        return;

    kDebug(TELEPATHY_DEBUG_AREA) << "Chat session closed";
}

void TelepathyChatSession::messageSent(const Tp::Message &message,
                                       Tp::MessageSendingFlags flags,
                                       const QString &sentMessageToken)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    Q_UNUSED(flags);
    Q_UNUSED(sentMessageToken);

    Kopete::Message::MessageType messageType = Kopete::Message::TypeNormal;
    /*
     if(message.messageType() == Tp::ChannelTypeMessageAction)
     {
      messageType = Kopete::Message::TypeAction;
     }
    */
    Kopete::Message newMessage(myself(), members());
    newMessage.setPlainBody(message.text());
    newMessage.setDirection(Kopete::Message::Outbound);
    newMessage.setType(messageType);

    appendMessage(newMessage);
    messageSucceeded();
}

void TelepathyChatSession::messageReceived(const Tp::ReceivedMessage &message)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    Kopete::Message::MessageType messageType = Kopete::Message::TypeNormal;

    Kopete::Message newMessage(members().first(), myself());
    newMessage.setPlainBody(message.text());
    newMessage.setDirection(Kopete::Message::Inbound);
    newMessage.setType(messageType);

    appendMessage(newMessage);
}

void TelepathyChatSession::pendingMessageRemoved(const Tp::ReceivedMessage &message)
{
    Q_UNUSED(message);

    // TODO: Implement me!
}

Tp::ChannelRequestPtr TelepathyChatSession::channelRequest()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return m_channelRequest;
}

void TelepathyChatSession::setTextChannel(Tp::TextChannelPtr textChannel)
{
    kDebug(TELEPATHY_DEBUG_AREA);
    m_textChannel = textChannel;

    // We must get the text channel ready with the required features.
    Tp::Features features;
    features << Tp::TextChannel::FeatureCore
             << Tp::TextChannel::FeatureMessageCapabilities
             << Tp::TextChannel::FeatureMessageQueue
             << Tp::TextChannel::FeatureMessageSentSignal;

    connect(m_textChannel->becomeReady(features),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onTextChannelReady(Tp::PendingOperation*)));
}

void TelepathyChatSession::onTextChannelReady(Tp::PendingOperation *op)
{
    if (TelepathyCommons::isOperationError(op))
        return;

    QObject::connect(m_textChannel.data(),
                     SIGNAL(messageSent(const Tp::Message &, Tp::MessageSendingFlags, const QString &)),
                     this,
                     SLOT(messageSent(const Tp::Message &, Tp::MessageSendingFlags, const QString &)));
    QObject::connect(m_textChannel.data(),
                     SIGNAL(messageReceived(const Tp::ReceivedMessage &)),
                     this,
                     SLOT(messageReceived(const Tp::ReceivedMessage &)));
    QObject::connect(m_textChannel.data(),
                     SIGNAL(pendingMessageRemoved(const Tp::ReceivedMessage &)),
                     this,
                     SLOT(pendingMessageRemoved(const Tp::ReceivedMessage &)));
}

Tp::PendingChannelRequest *TelepathyChatSession::pendingChannelRequest()
{
    return m_pendingChannelRequest;
}

