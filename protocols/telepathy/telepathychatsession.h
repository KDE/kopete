/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.co.uk>
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

#ifndef KOPETE_PROTOCOL_TELEPATHY_TELEPATHYCHATSESSION_H
#define KOPETE_PROTOCOL_TELEPATHY_TELEPATHYCHATSESSION_H

#include <kopetechatsession.h>
#include <kopetecontact.h>
#include <kopetemessage.h>

#include <QSharedPointer>

#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Message>
#include <TelepathyQt4/Constants>
#include <TelepathyQt4/ChannelRequest>
#include <TelepathyQt4/PendingChannelRequest>

namespace Telepathy
{
namespace Client {
class Contact;
class Channel;
}
}

class TelepathyChatSession : public Kopete::ChatSession
{
    Q_OBJECT

public:
    TelepathyChatSession(const Kopete::Contact *user, Kopete::ContactPtrList others, Kopete::Protocol *protocol);
    virtual ~TelepathyChatSession();

    void createTextChannel(QSharedPointer<Tp::Contact>);

    Tp::ChannelRequestPtr channelRequest();
    Tp::PendingChannelRequest *pendingChannelRequest();
    void setTextChannel(const Tp::TextChannelPtr &textChannel);

private slots:
    void sendMessage(Kopete::Message &);
    void onEnsureChannelFinished(Tp::PendingOperation*);
    void chatSessionRequestClose(Tp::PendingOperation*);
    void closingChatSession(Kopete::ChatSession *);
    void messageSent(const Tp::Message &message, Tp::MessageSendingFlags flags, const QString &sentMessageToken);
    void messageReceived(const Tp::ReceivedMessage &message);
    void pendingMessageRemoved(const Tp::ReceivedMessage &message);
    void onTextChannelReady(Tp::PendingOperation *op);


private:
    QSharedPointer<Tp::Contact> m_contact;
    QSharedPointer<Tp::TextChannel> m_channel;
    Tp::ChannelRequestPtr m_channelRequest;
    Tp::TextChannelPtr m_textChannel;
    Tp::PendingChannelRequest *m_pendingChannelRequest;
};


#endif  // Header guard

