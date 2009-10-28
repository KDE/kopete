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

#include <KopeteTelepathy/telepathychatsession.h>
#include <KopeteTelepathy/telepathyaccount.h>

#include <ui/kopeteview.h>
#include <kopetechatsessionmanager.h>

#include <KDebug>
#include <KLocale>
#include <KAction>
#include <KActionCollection>
#include <KMessageBox>

#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/ContactManager>

TelepathyChatSession::TelepathyChatSession(const Kopete::Contact *user, Kopete::ContactPtrList others, Kopete::Protocol *protocol)
        : Kopete::ChatSession(user, others, protocol),
        m_pendingChannelRequest(0)
{
    kDebug();
    Kopete::ChatSessionManager::self()->registerChatSession(this);

    QObject::connect(this, SIGNAL(messageSent(Kopete::Message&, Kopete::ChatSession*)), this, SLOT(sendMessage(Kopete::Message&)));

    KAction *shareMyDesktop = new KAction(KIcon("krfb"), i18n("S&hare My Desktop"), this);
    actionCollection()->addAction("shareMyDesktop", shareMyDesktop);
    shareMyDesktop->setShortcut(KShortcut(Qt::CTRL + Qt::Key_D));
    QObject::connect(shareMyDesktop,
                     SIGNAL(triggered(bool)),
                     SLOT(onShareMyDesktop()));

    setXMLFile ("telepathychatui.rc");
}

TelepathyChatSession::~TelepathyChatSession()
{
    kDebug();

    // When the ChatSession is closed, we should close the channel so that a new one is launched if
    // the same contact tries to contact us again.
    if (!m_textChannel.isNull()) {
        m_textChannel->requestClose();
        // FIXME: Can we connect the signal that this is done to the parent slot somewhere for error
        // handling?
    }
}

void TelepathyChatSession::createTextChannel(Tp::ContactPtr contact)
{
    kDebug();
    m_contact = contact;

    QString preferredHandler("org.freedesktop.Telepathy.Client.KopetePlugin");

    TelepathyAccount *telepathyAccount = qobject_cast<TelepathyAccount*>(account());
    if (!telepathyAccount) {
        kWarning() << "Null telepathy account. Fail.";
        return;
    }

    Tp::AccountPtr account = telepathyAccount->account();
    if (account.isNull()) {
        kWarning() << "Null account. Fail.";
        return;
    }

    m_pendingChannelRequest =
            account->ensureTextChat(contact, QDateTime::currentDateTime(), preferredHandler);

    m_channelRequest = m_pendingChannelRequest->channelRequest();
    kDebug() << "m_channelRequest:" << m_channelRequest.data();

    connect(m_pendingChannelRequest,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onEnsureChannelFinished(Tp::PendingOperation*)));
}

void TelepathyChatSession::onEnsureChannelFinished(Tp::PendingOperation* op)
{
    kDebug();

    if (op->isError()) {
        kWarning() << "Ensuring Channel Failed:" << op->errorName() << op->errorMessage();
        return;
    }

    m_channelRequest = m_pendingChannelRequest->channelRequest();
    m_pendingChannelRequest = 0;
}

void TelepathyChatSession::sendMessage(Kopete::Message &message)
{
    kDebug();

    if (!m_textChannel) {
        kWarning() << "Message not sent because channel does not yet exist.";

        // Indicate that the message sending failed.
        message.setState(Kopete::Message::StateError);
        appendMessage(message);

        // FIXME: Only call messageSucceeded() if there are no messages in the process of being sent.
        messageSucceeded();
        return;
    }

    m_textChannel->send(message.plainBody());
}

void TelepathyChatSession::messageSent(const Tp::Message &message,
                                       Tp::MessageSendingFlags flags,
                                       const QString &sentMessageToken)
{
    kDebug();

    Q_UNUSED(flags);
    Q_UNUSED(sentMessageToken);

    // FIXME: We need to process outgoing messages better, so we can display when they fail etc

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
    kDebug();

    Kopete::Message::MessageType messageType = Kopete::Message::TypeNormal;

    Kopete::Message newMessage(members().first(), myself());
    newMessage.setPlainBody(message.text());
    newMessage.setDirection(Kopete::Message::Inbound);
    newMessage.setType(messageType);

    appendMessage(newMessage);

    // Acknowledge the receipt of this message, so it won't be redespatched to us when we close
    // the channel.
    m_textChannel->acknowledge(QList<Tp::ReceivedMessage>() << message);
}

Tp::ChannelRequestPtr TelepathyChatSession::channelRequest()
{
    kDebug();
    return m_channelRequest;
}

void TelepathyChatSession::setTextChannel(const Tp::TextChannelPtr &textChannel)
{
    kDebug();
    m_textChannel = textChannel;

    // We must get the text channel ready with the required features.
    Tp::Features features;
    features << Tp::TextChannel::FeatureCore
             << Tp::TextChannel::FeatureMessageCapabilities
             << Tp::TextChannel::FeatureMessageQueue
             << Tp::TextChannel::FeatureMessageSentSignal;

    if (m_textChannel->isReady(features)) {
        kDebug() << "Already ready.";
    } else {
        kDebug() << "Not already ready.";
    }

    connect(m_textChannel->becomeReady(features),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onTextChannelReady(Tp::PendingOperation*)));
}

void TelepathyChatSession::onTextChannelReady(Tp::PendingOperation *op)
{
    kDebug();
    if (op->isError()) {
        kWarning() << "Text channel failed to become ready:"
                                       << op->errorName()
                                       << op->errorMessage();
        return;
    }

    QObject::connect(m_textChannel.data(),
                     SIGNAL(messageSent(const Tp::Message &, Tp::MessageSendingFlags, const QString &)),
                     this,
                     SLOT(messageSent(const Tp::Message &, Tp::MessageSendingFlags, const QString &)));
    QObject::connect(m_textChannel.data(),
                     SIGNAL(messageReceived(const Tp::ReceivedMessage &)),
                     this,
                     SLOT(messageReceived(const Tp::ReceivedMessage &)));

    // Check for messages already there in the message queue.
    foreach (const Tp::ReceivedMessage &message, m_textChannel->messageQueue()) {
        messageReceived(message);
    }
}

Tp::PendingChannelRequest *TelepathyChatSession::pendingChannelRequest()
{
    return m_pendingChannelRequest;
}

void TelepathyChatSession::onShareMyDesktop()
{
    QVariantMap req;
    req.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
               TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAM_TUBE);
    req.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
               Tp::HandleTypeContact);
    req.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandle"),
               m_contact->handle()[0]);
    req.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAM_TUBE ".Service"),
               "rfb");

    TelepathyAccount *telepathyAccount = qobject_cast<TelepathyAccount *>(account());
    Tp::AccountPtr account = telepathyAccount->account();
    QObject::connect(account->ensureChannel(req),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onEnsureShareDesktop(Tp::PendingOperation*)));
}

void TelepathyChatSession::onEnsureShareDesktop(Tp::PendingOperation *op)
{
    if (op->isError()) {
        kWarning() << "Ensuring channel failed:"
                   << op->errorName()
                   << op->errorMessage();

        if (op->errorName() == TELEPATHY_ERROR_DISCONNECTED) {
            KMessageBox::error(view()->mainWidget(),
                               i18n("An error occurred sharing your desktop. "
                                    "Please ensure that you have krfb installed."),
                               i18n("Error - Share My Desktop"));
        } else {
            KMessageBox::error(view()->mainWidget(),
                               i18n("An unknown error occurred sharing your desktop."),
                               i18n("Error - Share My Desktop"));
        }
    }
}

#include "telepathychatsession.moc"

