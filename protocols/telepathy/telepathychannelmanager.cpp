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

#include <telepathychannelmanager.h>

#include <telepathyaccount.h>
#include <telepathychatsession.h>
#include <telepathyclienthandler.h>
#include <telepathycontact.h>
#include <telepathyprotocolinternal.h>
#include <telepathyfiletransfer.h>

#include <KDebug>

#include <kopeteaccountmanager.h>
#include <kopetechatsessionmanager.h>
#include <kopetemetacontact.h>

#include <TelepathyQt4/Debug>
#include <TelepathyQt4/Feature>
#include <TelepathyQt4/PendingChannel>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/TextChannel>

#include <QSharedPointer>

struct TelepathyChannelManager::ChannelHandlingContext
{
    Tp::ChannelPtr channel;
    TelepathyClientHandler::HandleChannelsData *data;
};

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

    // Delete any pending handling contexts
    foreach (QList<ChannelHandlingContext> contactContexts, contexts.values()) {
        for (QList<ChannelHandlingContext>::iterator context = contactContexts.begin();
                context != contactContexts.end(); ++context) {
            context->data->context->setFinishedWithError(TELEPATHY_ERROR_NOT_AVAILABLE,
                    "Channel manager destroyed before handling completed");
            delete context->data;
            context->data = 0;
        }
    }

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

    if (data->channels.size() != 1) {
        // With the previous code, this was apparently supported (looping through the channels and
        // handling them one by one), but actually it wasn't - if there had been multiple
        // channels, data->context->setFinished() would've been called multiple times, etc.
        kDebug() << "Got multiple channels to handle - currently not supported";
        data->context->setFinishedWithError(TELEPATHY_ERROR_NOT_IMPLEMENTED,
                "Handling multiple channels is currently not implemented");
        return;
    }

    Tp::ChannelPtr channel = data->channels[0];
    QVariantMap properties = channel->immutableProperties();

    QString contactId;
    if (properties[TELEPATHY_INTERFACE_CHANNEL ".Requested"].toBool())
        contactId = properties[TELEPATHY_INTERFACE_CHANNEL ".TargetID"].toString();
    else
        contactId = properties[TELEPATHY_INTERFACE_CHANNEL ".InitiatorID"].toString();

    TelepathyContact *contact = getTpContact(data->account, contactId);

    if (!contact) {
        kDebug() << "Failed to get the contact, deleted account?";
        data->context->setFinishedWithError(TELEPATHY_ERROR_NOT_AVAILABLE,
                "failed to get internal contact object");
        delete data;
        return;
    }

    if (!contexts.contains(contact)) {
        // -> Not currently fetching the internal contact
        QObject::connect(contact, SIGNAL(internalContactFetched(bool)),
                this, SLOT(onInternalContactFetched(bool)));
        contact->fetchInternalContact();
    }

    ChannelHandlingContext context = {channel, data};
    contexts[contact].push_back(context);
}

void TelepathyChannelManager::onInternalContactFetched(bool success)
{
    TelepathyContact *contact = qobject_cast<TelepathyContact *>(sender());

    kDebug();

    if (!contexts.contains(contact)) {
        kDebug() << "Got unexpected internalContactFetched(), ignoring";
        return;
    }

    QList<ChannelHandlingContext> &contactContexts = contexts[contact]; 
    for (QList<ChannelHandlingContext>::iterator context = contactContexts.begin();
            context != contactContexts.end(); ++context) {
        Tp::ChannelPtr channel = context->channel;
        QVariantMap properties = channel->immutableProperties();

        if (success) {
            // Check the channel type
            if (properties[TELEPATHY_INTERFACE_CHANNEL ".ChannelType"] ==
                    TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT) {
                kDebug() << "Handling:" << TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT;
                handleTextChannel(channel, contact, context->data);
            } else if (properties[TELEPATHY_INTERFACE_CHANNEL ".ChannelType"] ==
                    TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER) {
                kDebug() << "Handling:" << TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER;
                handleFileTransferChannel(channel, contact, context->data);
            }
        } else {
            kDebug() << "Failure";
            context->data->context->setFinishedWithError(TELEPATHY_ERROR_NOT_AVAILABLE,
                    "failed to fetch internal contact");
        }

        delete context->data;
    }

    QObject::disconnect(contact, SIGNAL(onInternalContactFetched(bool)),
            this, SLOT(onInternalContactFetched(bool)));
    contexts.remove(contact);

    kDebug() << "onInternalContactFetched() finished.";
}

void TelepathyChannelManager::handleTextChannel(Tp::ChannelPtr channel,
                                                TelepathyContact *contact,
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

        data->context->setFinished();
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

    data->context->setFinished();
}

void TelepathyChannelManager::handleFileTransferChannel(Tp::ChannelPtr channel,
                                                        TelepathyContact *contact,
                                                        TelepathyClientHandler::HandleChannelsData *data)
{
    kDebug();

    QVariantMap properties = channel->immutableProperties();

    if (properties[TELEPATHY_INTERFACE_CHANNEL ".Requested"].toBool()) {
        kDebug() << "Outgoing";

        QString fileName =
            properties[TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER ".Filename"].toString();

        (void *) new TelepathyFileTransfer(channel, contact, fileName);
    } else {
        kDebug() << "Incoming";
        (void *) new TelepathyFileTransfer(channel, contact);
    }

    data->context->setFinished();
}

TelepathyContact *TelepathyChannelManager::getTpContact(Tp::AccountPtr account,
                                                        const QString &contactId)
{
    QList<Kopete::Account*> kAccounts = Kopete::AccountManager::self()->
        accounts(TelepathyProtocolInternal::protocolInternal()->protocol());
    TelepathyAccount *tpAccount = 0L;

    foreach (Kopete::Account *kAccount, kAccounts) {
        TelepathyAccount *maybeTpAccount = qobject_cast<TelepathyAccount*>(kAccount);

        if (!maybeTpAccount)
            continue;

        if (maybeTpAccount->account()->uniqueIdentifier() == account->uniqueIdentifier()) {
            tpAccount = maybeTpAccount;
            break;
        }
    }

    if (!tpAccount) {
        kDebug() << "Failed to find the corresponding account!";
        return 0L;
    }

    // We have the same account.
    QHash<QString, Kopete::Contact*> contacts = tpAccount->contacts();
    TelepathyContact *tpContact = 0L;

    for (QHash<QString, Kopete::Contact *>::const_iterator i  = contacts.constBegin();
                                                           i != contacts.constEnd();
                                                           ++i) {
        // Check its a tp contact
        TelepathyContact *contact = qobject_cast<TelepathyContact*>(i.value());

        if (!contact) {
            kWarning() << "Not a TelepathyContact";
            continue;
        }

        // FIXME: String ID comparison is WRONG!
        if (contact->contactId() == contactId) {
            kDebug() << "Found the remote contact.";
            tpContact = contact;
            break;
        }
    }

    if (!tpContact) {
        // No contact found == channel came from somebody we don't have in our contact list, let's
        // create a temporary contact.

        Kopete::MetaContact *metaContact = new Kopete::MetaContact();
        metaContact->setTemporary(true);

        if (tpAccount->addContact(contactId, metaContact))
            tpContact = qobject_cast<TelepathyContact *>(tpAccount->contacts()[contactId]);
    }

    return tpContact;
}

#include "telepathychannelmanager.moc"

