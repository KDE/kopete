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

#include <KopeteTelepathy/telepathychannelmanager.h>

#include <KopeteTelepathy/telepathyaccount.h>
#include <KopeteTelepathy/telepathychatsession.h>
#include <KopeteTelepathy/telepathyclienthandler.h>
#include <KopeteTelepathy/telepathycontact.h>
#include <KopeteTelepathy/telepathyprotocolinternal.h>
#include <KopeteTelepathy/telepathyfiletransfer.h>

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
    kDebug();

    // Set up the singleton instance
    s_self = this;

    // Register the Telepathy Channel Handler
    m_clientRegistrar = Tp::ClientRegistrar::create();
    m_clientRegistrar->registerClient(Tp::AbstractClientPtr(m_clientHandler),
                                      "KopetePluginHandler");

    // Enable Telepathy Debug
    Tp::enableDebug(true);
}

TelepathyChannelManager::~TelepathyChannelManager()
{
    kDebug();

    // Delete the DBus adaptors
    delete m_clientHandler;

    // Delete the singleton instance of this class
    s_self = 0;
}

TelepathyChannelManager *TelepathyChannelManager::instance()
{
    kDebug();

    // Construct the singleton if hasn't been already
    if (!s_self)
        s_self = new TelepathyChannelManager(0);

    // Return the singleton instance of this class
    return s_self;
}

void TelepathyChannelManager::handleChannels(TelepathyClientHandler::HandleChannelsData *data)
{
    kDebug();

    // This method is called by TelepathyClientHandler::handleChannels()
    // when there are channels to be handled, with the HandleChannelsData
    // struct containing shared pointers for all the relevant data we need
    // to handle the channel.

    // Loop through all the channels that are available in this
    // batch to be handled.
    foreach (const Tp::ChannelPtr channel, data->channels) {

        QVariantMap properties = channel->immutableProperties();

        // Check the channel type
        if (properties[TELEPATHY_INTERFACE_CHANNEL ".ChannelType"] ==
            TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT) {

            kDebug() << "Handling:" << TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT;
            handleTextChannel(channel, data);
        }
        else if (properties[TELEPATHY_INTERFACE_CHANNEL ".ChannelType"] ==
                 TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER) {

            kDebug() << "Handling:" << TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER;
            handleFileTransferChannel(channel, data);
        }
    }

    data->context->setFinished();
    delete data;

    kDebug() << "handleCHannels finished.";
}

void TelepathyChannelManager::handleTextChannel(Tp::ChannelPtr channel,
                                                TelepathyClientHandler::HandleChannelsData *data)
{
    kDebug();

    Tp::TextChannelPtr textChannel =
        Tp::TextChannelPtr(qobject_cast<Tp::TextChannel*>(channel.data()));

    QVariantMap properties = channel->immutableProperties();

    // Check to see if this channel satisfies a request that was made by this
    // program, or if it was initiated by the contact at the other end.
    if (properties[TELEPATHY_INTERFACE_CHANNEL ".Requested"] == false) {
        kDebug() << "Text Channel initiated by remote contact.";

        // Get KopeteContact
        TelepathyContact *contact = getTpContact(data->account,
                properties[TELEPATHY_INTERFACE_CHANNEL ".InitiatorID"].toString());

        if (!contact) {
            kWarning(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT) << "Contact not found!";
            channel->requestClose();
            return;
        }

        Kopete::ContactPtrList chatContacts;
        chatContacts << contact;

        // Look for already created chat session
        Kopete::ChatSession * chatSession =
                Kopete::ChatSessionManager::self()->findChatSession(contact->account()->myself(),
                                                                    chatContacts,
                                                                    contact->account()->protocol());

        TelepathyChatSession *tpChatSession = 0L;

        // Check if it's a telepathy chat session
        if (chatSession)
            tpChatSession = qobject_cast<TelepathyChatSession *>(chatSession);

        // Create a new telepathy chat session
        if (!tpChatSession)
            tpChatSession = new TelepathyChatSession(contact->account()->myself(),
                                                     chatContacts,
                                                     contact->account()->protocol());

        // Set up text channel in the chat session
        tpChatSession->setTextChannel(
                Tp::TextChannelPtr(qobject_cast<Tp::TextChannel*>(channel.data())));

        return;
    }

    kDebug() << "Channel initiated locally.";

    foreach (Kopete::ChatSession *session,
             Kopete::ChatSessionManager::self()->sessions()) {
        TelepathyChatSession *tpc = qobject_cast<TelepathyChatSession*>(session);

        if (!tpc)
            continue;

        foreach (Tp::ChannelRequestPtr crp, data->requestsSatisfied) {
            if (tpc->pendingChannelRequest() &&
                tpc->pendingChannelRequest()->channelRequest())
                if (tpc->pendingChannelRequest()->
                    channelRequest()->userActionTime() == crp->userActionTime()) {
                    tpc->setTextChannel(textChannel);
                    break;
                }
        }
    }
}

void TelepathyChannelManager::handleFileTransferChannel(Tp::ChannelPtr channel,
                                                        TelepathyClientHandler::HandleChannelsData *data)
{
    kDebug();

    QVariantMap properties = channel->immutableProperties();

    // Check to see if this channel satisfies a request that was made by this
    // program, or if it was initiated by the contact at the other end.
    if (properties[TELEPATHY_INTERFACE_CHANNEL ".Requested"] == false) {
        kDebug() << "Incoming file transfer channel.";

        TelepathyContact *contact = getTpContact(data->account,
                properties[TELEPATHY_INTERFACE_CHANNEL ".InitiatorID"].toString());

        if (!contact) {
            kWarning(TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER) << "Contact not found!";
            channel->requestClose();
            return;
        }

        (void *) new TelepathyFileTransfer(channel, contact);
    }
    else {
        kDebug() << "Outgoing file transfer channel.";

        TelepathyContact *contact = getTpContact(data->account,
                properties[TELEPATHY_INTERFACE_CHANNEL ".TargetID"].toString());

        if (!contact) {
            kWarning(TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER) << "Contact not found!";
            channel->requestClose();
            return;
        }

        QString fileName =
            properties[TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER ".Filename"].toString();

        (void *) new TelepathyFileTransfer(channel, contact, fileName);
    }
}

TelepathyContact *TelepathyChannelManager::getTpContact(Tp::AccountPtr account,
                                                        const QString &contactId)
{
    TelepathyContact *tpContact = 0L;
    QList<Kopete::Account*> kAccounts = Kopete::AccountManager::self()->
        accounts(TelepathyProtocolInternal::protocolInternal()->protocol());

    foreach (Kopete::Account *kAccount, kAccounts) {
             TelepathyAccount *tpAccount = qobject_cast<TelepathyAccount*>(kAccount);

        if (!tpAccount)
            continue;

        if (tpAccount->account()->uniqueIdentifier() ==
            account->uniqueIdentifier()) {

            // We have the same account.
            QHash<QString, Kopete::Contact*> contacts = tpAccount->contacts();

            QHash<QString, Kopete::Contact*>::const_iterator contactIterator =
                contacts.constBegin();

            while (contactIterator != contacts.constEnd()) {

                // Check its a tp contact
                TelepathyContact *contact = qobject_cast<TelepathyContact*>(
                        contactIterator.value());

                if (!contact) {
                    kWarning() << "Not a TelepathyContact";
                    ++contactIterator;
                    continue;
                }

                // If we have any invalid contacts in contactslist.xml that
                // don't exist in the tp roster, we will have a null
                // internalContact(), so skip over them here to avoid a crash.
                if (!contact->internalContact()) {
                    kWarning() << "Skipping over contact in Kopete contact \
                        list which is not in telepathy roster.";
                    ++contactIterator;
                    continue;
                }

                if (contact->internalContact()->id() == contactId) {
                    kDebug() << "Found the remote contact.";
                    tpContact = contact;
                    break;
                }

                ++contactIterator;
            }
        }
    }

    return tpContact;
}

#include "telepathychannelmanager.moc"

