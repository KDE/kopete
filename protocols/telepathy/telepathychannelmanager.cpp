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

#include "telepathychannelmanager.h"

#include "telepathyaccount.h"
#include "telepathychatsession.h"
#include "telepathyclienthandler.h"
#include "telepathycontact.h"
#include "telepathyprotocol.h"
#include "common.h"

#include <KDebug>

#include <kopeteaccountmanager.h>
#include <kopetechatsessionmanager.h>

#include <TelepathyQt4/Debug>
#include <TelepathyQt4/Feature>
#include <TelepathyQt4/PendingChannel>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/TextChannel>

TelepathyChannelManager* TelepathyChannelManager::s_self = 0;

TelepathyChannelManager::TelepathyChannelManager(QObject *parent)
 : QObject(parent),
   m_clientHandler(new TelepathyClientHandler)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    // Set up the singleton instance
    s_self = this;

    // Register the Telepathy Channel Handler
    m_clientRegistrar = Tp::ClientRegistrar::create();
    m_clientRegistrar->registerClient(Tp::AbstractClientPtr(m_clientHandler), "KopetePluginHandler");

    // Enable Telepathy Debug
    Tp::enableDebug(true);
}

TelepathyChannelManager::~TelepathyChannelManager()
{
    kDebug(TELEPATHY_DEBUG_AREA);

    // Delete the DBus adaptors
    delete m_clientHandler;

    // Delete the singleton instance of this class
    s_self = 0;
}

TelepathyChannelManager *TelepathyChannelManager::instance()
{
    kDebug(TELEPATHY_DEBUG_AREA);

    // Construct the singleton if hasn't been already
    if (!s_self) {
        s_self = new TelepathyChannelManager(0);
    }

    // Return the singleton instance of this class
    return s_self;
}

// FIXME: The HandleChannelsData*'s are being leaked here. This should be fixed when this method
// and the following one get rearchitected to be made of less fail.
void TelepathyChannelManager::handleChannels(TelepathyClientHandler::HandleChannelsData *data)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    // This method is called by TelepathyClientHandler::handleChannels() when there are channels
    // to be handled, with the HandleChannelsData struct containing shared pointers for all the
    // relevant data we need to handle the channel.

    // Loop through all the channels that are available in this batch to be handled.
    foreach (const Tp::ChannelPtr channel, data->channels) {

        // Check if this channel is a TextChannel, and skip handling it if it isn't.
        Tp::TextChannelPtr textChannel =
                Tp::TextChannelPtr(qobject_cast<Tp::TextChannel*>(channel.data()));

        if (!textChannel) {
            kDebug(TELEPATHY_DEBUG_AREA) << "Channel is of non-text type. Ignoring.";
            continue;
        }

        kDebug() << "Handling Text Channel.";

        // Check to see if this channel satisfies a request that was made by this program, or if
        // it was initiated by the contact at the other end.
        if (data->requestsSatisfied.size() == 0) {
            kDebug(TELEPATHY_DEBUG_AREA) << "Text Channel initiated by remote contact.";

            // Get the text channel ready
            Tp::PendingReady *pr = textChannel->becomeReady(Tp::Channel::FeatureCore);
            m_channelToHandleChannelData.insert(pr, qMakePair(textChannel, data));

            connect(pr,
                    SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onChannelReady(Tp::PendingOperation*)));
            continue;
        }

        kDebug(TELEPATHY_DEBUG_AREA) << "Channel initiated locally.";

        foreach (Kopete::ChatSession *session, Kopete::ChatSessionManager::self()->sessions()) {
            TelepathyChatSession *tpc = qobject_cast<TelepathyChatSession*>(session);
            if (!tpc) {
                continue;
            }

            foreach (Tp::ChannelRequestPtr crp, data->requestsSatisfied) {
                if (tpc->pendingChannelRequest()->channelRequest()) {
                    if (tpc->pendingChannelRequest()->channelRequest()->userActionTime() == crp->userActionTime()) {
                        tpc->setTextChannel(textChannel);
                        break;
                    }
                }
            }
        }
    }

    kDebug(TELEPATHY_DEBUG_AREA) << "Check if the context is finished.";

    int count = 0;
    QMap<Tp::PendingReady*, QPair<Tp::TextChannelPtr, TelepathyClientHandler::HandleChannelsData*> >::const_iterator i =
            m_channelToHandleChannelData.constBegin();

    while (i != m_channelToHandleChannelData.constEnd()) {
        if (i.value().second->context.data() == data->context.data()) {
            ++count;
        }
        ++i;
    }

    if (count == 0) {
        kDebug(TELEPATHY_DEBUG_AREA) << "Count is 0. Destroy data object.";
        data->context->setFinished();
    }
    kDebug(TELEPATHY_DEBUG_AREA) << "handleCHannels finished.";
}

void TelepathyChannelManager::onChannelReady(Tp::PendingOperation *op)
{
    if (TelepathyCommons::isOperationError(op)) {
        kDebug(TELEPATHY_DEBUG_AREA) << "Argh badgers!" << op->errorName() <<op->errorMessage();
        return;
    }

    Tp::TextChannelPtr textChannel =
            m_channelToHandleChannelData.value(qobject_cast<Tp::PendingReady*>(op)).first;
    if (!textChannel) {
        kWarning(TELEPATHY_DEBUG_AREA) << "Not a TextChannel.";
        return;
    }

    TelepathyClientHandler::HandleChannelsData *data = m_channelToHandleChannelData.value(qobject_cast<Tp::PendingReady*>(op)).second;
    m_channelToHandleChannelData.remove(qobject_cast<Tp::PendingReady*>(op));

    kDebug(TELEPATHY_DEBUG_AREA) << data->account
                                 << data->userActionTime
                                 << data->connection;

    kDebug(TELEPATHY_DEBUG_AREA) << data;

    kDebug(TELEPATHY_DEBUG_AREA) << "Case:" << textChannel;

    if (!data) {
        kWarning(TELEPATHY_DEBUG_AREA) << "Argh badgers.";
    }

    kDebug(TELEPATHY_DEBUG_AREA) << "Other side started this channel.";

    QList<Kopete::Account*> kAccounts =
            Kopete::AccountManager::self()->accounts(TelepathyProtocol::protocol());
    foreach (Kopete::Account *kAccount, kAccounts) {
        TelepathyAccount *tpAccount = qobject_cast<TelepathyAccount*>(kAccount);

        if (!tpAccount) {
            continue;
        }

        if (tpAccount->account()->uniqueIdentifier() == data->account->uniqueIdentifier()) {

            // We have the same account.
            QHash<QString, Kopete::Contact*> contacts = tpAccount->contacts();
            Kopete::ContactPtrList others;

            QHash<QString, Kopete::Contact*>::const_iterator contactIterator = contacts.constBegin();
            while (contactIterator != contacts.constEnd()) {

                // Check its a tp contact
                TelepathyContact *other = qobject_cast<TelepathyContact*>(contactIterator.value());

                if (!other) {
                    kWarning(TELEPATHY_DEBUG_AREA) << "Not a TelepathyContact";
                    ++contactIterator;
                    continue;
                }

                // If we have any invalid contacts in contactslist.xml that don't exist in the tp
                // roster, we will have a null internalContact(), so skip over them here to avoid
                // a crash.
                if (!other->internalContact()) {
                    kWarning(TELEPATHY_DEBUG_AREA) << "Skipping over contact in Kopete contact list which is not in telepathy roster.";
                    ++contactIterator;
                    continue;
                }

                // See if it is in the list of contacts for this channel.
                if (other->internalContact()->alias() == textChannel->initiatorContact()->alias()) {
                    kDebug(TELEPATHY_DEBUG_AREA) << "Found the remote contact.";
                    others << (other);
                }

                ++contactIterator;
            }

            Kopete::ChatSession * _manager =
                    Kopete::ChatSessionManager::self()->findChatSession(tpAccount->myself(),
                                                                        others,
                                                                        tpAccount->protocol());
            TelepathyChatSession *s = 0;
            if (_manager) {
                s = qobject_cast<TelepathyChatSession*>(_manager);
            }

            if (!s) {
                s = new TelepathyChatSession(tpAccount->myself(),
                                             others,
                                             tpAccount->protocol());
            }

            s->setTextChannel(textChannel);

            // Decide whether to set the context as finished.
            int count = 0;
            QMap<Tp::PendingReady*, QPair<Tp::TextChannelPtr, TelepathyClientHandler::HandleChannelsData*> >::const_iterator i =
                    m_channelToHandleChannelData.constBegin();

            while (i != m_channelToHandleChannelData.constEnd()) {
                if (i.value().second->context.data() == data->context.data()) {
                    ++count;
                }
                ++i;
            }

            if (count == 0) {
                data->context->setFinished();
            }
            return;
        }
    }
    kWarning() << "Incoming channel from an unknown account. We shouldn't get here :(";
}


#include "telepathychannelmanager.moc"

