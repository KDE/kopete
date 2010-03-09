/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
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

#include <telepathycontact.h>

#include <telepathyaccount.h>
#include <telepathychatsession.h>
#include <telepathyprotocolinternal.h>
#include <telepathyfiletransfer.h>

#include <KAction>
#include <KDebug>
#include <KFileDialog>
#include <KLocale>
#include <KMessageBox>

#include <kopetechatsession.h>
#include <kopetechatsessionmanager.h>
#include <kopetemetacontact.h>
#include <kopeteavatarmanager.h>
#include <kopetecontactlist.h>
#include <kopetegroup.h>
#include <kopeteuiglobal.h>

#include <TelepathyQt4/Contact>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/PendingContacts>

#include <QtCore/QPointer>

class TelepathyContact::TelepathyContactPrivate
{
public:
    TelepathyContactPrivate()
    : sync(true), fetchingContact(false)
    {
    }

    Tp::ContactPtr internalContact;
    QPointer<Kopete::ChatSession> currentChatSession;
    bool sync;
    bool fetchingContact;
};

TelepathyContact::TelepathyContact(TelepathyAccount *account, const QString &contactId,
                                   Kopete::MetaContact *parent)
        : Kopete::Contact(account, contactId, parent), d(new TelepathyContactPrivate)
{
    kDebug();
    setOnlineStatus(TelepathyProtocolInternal::protocolInternal()->Offline);
    setFileCapable(true);
}

TelepathyContact::~TelepathyContact()
{
    delete d;
}

bool TelepathyContact::isReachable()
{
    bool ret = account()->isConnected() && (!internalContact().isNull());
    kDebug() << ret;

    return account()->isConnected() && (!internalContact().isNull());
}

void TelepathyContact::serialize(QMap< QString, QString >& serializedData,
                                 QMap< QString, QString >& addressBookData)
{
    //kDebug();

    Q_UNUSED(serializedData);
    Q_UNUSED(addressBookData);
}

QList<KAction *> *TelepathyContact::customContextMenuActions()
{
    kDebug();

    QList<KAction*> *actions = new QList<KAction*>();

    if (!internalContact())
        return actions;

    struct ActionInfo {
        KIcon icon;
        QString text;
        bool enabled;
        const char *slot;
    } infos[3] = {
        {
            KIcon("mail-reply-sender"),
            i18n("(Re)request Presence Authorization"),
            internalContact()->subscriptionState() == Tp::Contact::PresenceStateNo,
            SLOT(requestAuthorization())
        },
        {
            KIcon("mail-forward"),
            i18n("Send Presence Authorization"),
            internalContact()->publishState() == Tp::Contact::PresenceStateAsk,
            SLOT(sendAuthorization()),
        },
        {
            KIcon("edit-delete"),
            i18n("Remove Presence Authorization"),
            internalContact()->publishState() != Tp::Contact::PresenceStateNo,
            SLOT(removeAuthorization())
        }
    };

    for (int i = 0; i < 3; i++) {
        KAction *action = new KAction(this);

        action->setIcon(infos[i].icon);
        action->setText(infos[i].text);
        action->setEnabled(infos[i].enabled);

        connect(action, SIGNAL(triggered(bool)), this, infos[i].slot);

        actions->push_back(action);
    }

    return actions;
}

QList<KAction *> *TelepathyContact::customContextMenuActions(Kopete::ChatSession *)
{
    return customContextMenuActions();
}

Kopete::ChatSession *TelepathyContact::manager(CanCreateFlags canCreate)
{
    kDebug();

    Kopete::ContactPtrList members;
    members << this;

    return manager(members, canCreate);
}

Kopete::ChatSession *TelepathyContact::manager(Kopete::ContactPtrList members, CanCreateFlags canCreate)
{
    kDebug();

    if (d->currentChatSession.isNull()) {
        kDebug();

        // Try to find existing chat session
        Kopete::ChatSession *existingSession =
                Kopete::ChatSessionManager::self()->findChatSession(account()->myself(),
                                                                    members,
                                                                    account()->protocol());
        if (existingSession) {
            kDebug() << "chat exist";
            d->currentChatSession = existingSession;
        } else if (canCreate == Kopete::Contact::CanCreate) {
            kDebug() << "chat not exist";
            TelepathyChatSession *newSession = new TelepathyChatSession(account()->myself(),
                                                                        members,
                                                                        account()->protocol());
            newSession->createTextChannel(internalContact());
            d->currentChatSession = newSession;
        }
    }

    return d->currentChatSession;
}

void TelepathyContact::sync(unsigned int flags)
{
    if (!account()->isConnected() ||
        metaContact()->isTemporary() ||
        metaContact() == Kopete::ContactList::self()->myself() ||
        d->sync != true)
        return;

    if ((flags & Kopete::Contact::MovedBetweenGroup) ==
        Kopete::Contact::MovedBetweenGroup) {
        Kopete::GroupList kGroupList = metaContact()->groups();
        QStringList rGroupList = d->internalContact->groups();

        foreach (Kopete::Group *kgroup, kGroupList) {
            if (!rGroupList.contains(kgroup->displayName()) &&
                kgroup != Kopete::Group::topLevel()) {
                d->internalContact->addToGroup(kgroup->displayName());
            }
        }

        foreach (QString rgroup, rGroupList) {
            bool found = false;
            foreach (Kopete::Group *kgroup, kGroupList) {
                if (kgroup->displayName() == rgroup) {
                    found = true;
                    break;
                }
            }

            if (!found)
                d->internalContact->removeFromGroup(rgroup);
        }
    }
}

void TelepathyContact::sendFile(const KUrl &sourceURL, const QString &fileName,
                                uint fileSize)
{
    kDebug();

    Q_UNUSED(fileName);
    Q_UNUSED(fileSize);

    QString filePath;

    if (sourceURL.isValid())
        filePath = sourceURL.path(KUrl::RemoveTrailingSlash);
    else
        filePath = KFileDialog::getOpenFileName(KUrl(), "*", 0L,
                                                i18n("Kopete File Transfer"));

    QFileInfo file(filePath);

    if (file.exists()) {
        kDebug() << "Offering file:" << filePath;

        QVariantMap req;
        req.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
                   TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER);
        req.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
                   (uint) Tp::HandleTypeContact);
        req.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandle"),
                   d->internalContact->handle()[0]);

        // FIXME - We should not send the file path but the file basename.
        req.insert(QLatin1String(
                    TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER ".Filename"),
                    filePath);

        req.insert(QLatin1String(
                    TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER ".Size"),
                   (qulonglong) file.size());
        req.insert(QLatin1String(
                    TELEPATHY_INTERFACE_CHANNEL_TYPE_FILE_TRANSFER ".ContentType"),
                   "application/octet-stream");

        TelepathyAccount *telepathyAccount = qobject_cast<TelepathyAccount *>(account());
        Tp::AccountPtr account = telepathyAccount->account();
        account->ensureChannel(req, QDateTime::currentDateTime());
    }
}

void TelepathyContact::setInternalContact(Tp::ContactPtr contact)
{
    kDebug();

    if (d->internalContact) {
        Tp::Client::ConnectionInterfaceAvatarsInterface *avatarIface =
            d->internalContact->manager()->connection()->avatarsInterface();

        if (avatarIface)
            avatarIface->disconnect(this);
        d->internalContact->disconnect(this);
    }

    d->internalContact = contact;

    setOnlineStatus(TelepathyProtocolInternal::protocolInternal()->telepathyStatusToKopete(
            static_cast<Tp::ConnectionPresenceType>(contact->presenceType())));
    setNickName(contact->alias());
    setStatusMessage(contact->presenceMessage());

    /* Fetch groups from server and update local entries */
    serverToLocalSync();

    connect(contact.data(),
            SIGNAL(aliasChanged(const QString &)),
            SLOT(onAliasChanged(const QString &)));
    connect(contact.data(),
            SIGNAL(avatarTokenChanged(const QString &)),
            SLOT(onAvatarTokenChanged(const QString &)));
    connect(contact.data(),
            SIGNAL(simplePresenceChanged(const QString &, uint, const QString &)),
            SLOT(onSimplePresenceChanged(const QString &, uint, const QString &)));
    connect(contact.data(),
            SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
            SLOT(onSubscriptionStateChanged(Tp::Contact::PresenceState)));
    connect(contact.data(),
            SIGNAL(publishStateChanged(Tp::Contact::PresenceState)),
            SLOT(onPublishStateChanged(Tp::Contact::PresenceState)));
    connect(contact.data(),
            SIGNAL(blockStatusChanged(bool)),
            SLOT(onBlockStatusChanged(bool)));

    Tp::Client::ConnectionInterfaceAvatarsInterface *avatarIface =
            d->internalContact->manager()->connection()->avatarsInterface();

    if (avatarIface)
        connect(avatarIface,
                SIGNAL(AvatarRetrieved(uint, const QString&,
                        const QByteArray&, const QString&)),
                SLOT(onAvatarRetrieved(uint, const QString&,
                        const QByteArray&, const QString&)));
}

void TelepathyContact::fetchInternalContact()
{
    if (d->fetchingContact) {
        kDebug() << "Internal contact fetch already in progress, ignoring";
        return;
    }

    if (d->internalContact) {
        kDebug() << "Already have internal contact";
        QMetaObject::invokeMethod(this, "internalContactFetched", Qt::QueuedConnection, Q_ARG(bool, true));
        return;
    }

    Tp::AccountPtr tpAccount = static_cast<TelepathyAccount *>(account())->account();

    if (!tpAccount || !tpAccount->connection() || !tpAccount->connection()->isReady()) {
        kDebug() << "There is no ready connection for this contact, aborting";
        QMetaObject::invokeMethod(this, "internalContactFetched", Qt::QueuedConnection, Q_ARG(bool, false));
        return;
    }

    d->fetchingContact = true;

    Tp::ContactManager *contactManager = tpAccount->connection()->contactManager();

    QSet<Tp::Contact::Feature> features;
    features << Tp::Contact::FeatureAlias
             << Tp::Contact::FeatureAvatarToken
             << Tp::Contact::FeatureSimplePresence;

    connect(contactManager->contactsForIdentifiers(QStringList() << contactId(), features),
            SIGNAL(finished(Tp::PendingOperation*)),
            this,
            SLOT(onContactFetched(Tp::PendingOperation*)));
}

QString TelepathyContact::storedAvatarToken() const
{
    return property(TelepathyProtocolInternal::protocolInternal()->
            propAvatarToken).value().toString();
}

QString TelepathyContact::storedAvatarPath() const
{
    return property(TelepathyProtocolInternal::protocolInternal()->
            propPhoto).value().toString();
}

void TelepathyContact::onAliasChanged(const QString &alias)
{
    kDebug() << alias;
    setNickName(alias);
}

void TelepathyContact::onAvatarTokenChanged(const QString &avatarToken)
{
    kDebug() << avatarToken;

    if (storedAvatarToken() != avatarToken) {

        Tp::Client::ConnectionInterfaceAvatarsInterface *avatarIface =
                d->internalContact->manager()->connection()->avatarsInterface();

        if (avatarIface) {
            Tp::UIntList id;
            id.append(d->internalContact->handle()[0]);
            avatarIface->RequestAvatars(id);
        }
    }
}

void TelepathyContact::onAvatarRetrieved(uint contact, const QString& token,
        const QByteArray& avatar, const QString& type)
{
    Q_UNUSED(type);

    kDebug() << contact << token;

    if (contact != d->internalContact->handle()[0])
        return;

    // Is it needed? It's already checked in ContactManager
    if (storedAvatarToken() != token ||
        QFile::exists(storedAvatarPath()) == false) {

        removeProperty(
                TelepathyProtocolInternal::protocolInternal()->propPhoto);
        removeProperty(
                TelepathyProtocolInternal::protocolInternal()->propAvatarToken);

        QImage contactPhoto;
        contactPhoto.loadFromData(avatar);

        Kopete::AvatarManager::AvatarEntry entry;
        entry.name = contactId();
        entry.image = contactPhoto;
        entry.category = Kopete::AvatarManager::Contact;
        entry.contact = this;
        entry = Kopete::AvatarManager::self()->add(entry);

        // Save the image to the disk, then set the property.
        if(!entry.dataPath.isNull()) {
            setProperty(TelepathyProtocolInternal::protocolInternal()->
                    propPhoto, entry.dataPath);
            setProperty(TelepathyProtocolInternal::protocolInternal()->
                    propAvatarToken, token);
        }
    }
}

void TelepathyContact::onSimplePresenceChanged(const QString &status, uint type, const QString &presenceMessage)
{
    kDebug() << status << type << presenceMessage;
    setOnlineStatus(TelepathyProtocolInternal::protocolInternal()->telepathyStatusToKopete(
            static_cast<Tp::ConnectionPresenceType>(type)));
    setStatusMessage(Kopete::StatusMessage(presenceMessage));
}

void TelepathyContact::onSubscriptionStateChanged(Tp::Contact::PresenceState state)
{
    kDebug();

    if (state == Tp::Contact::PresenceStateNo) {
        // Contact refused to authorize / removed us
        if (KMessageBox::warningYesNo(Kopete::UI::Global::mainWidget(),
                    i18n(
                        "The contact %1 disallowed %2 from receiving his/her presence. "
                        "This account will no longer be able to view his/her online status. "
                        "Do you want to delete the contact?",
                        contactId(), account()->myself()->contactId()),
                    i18n("Notification"), KStandardGuiItem::del(), KGuiItem(i18n("Keep")))
                == KMessageBox::Yes) {
            deleteContact();

            if (!metaContact()->isTemporary()) {
                metaContact()->removeContact(this);
                if (metaContact()->contacts().isEmpty())
                    Kopete::ContactList::self()->removeMetaContact(metaContact());

                deleteLater();
            }
        } else {
            // We don't know anymore!
            setOnlineStatus(Kopete::OnlineStatus::Unknown);
            setStatusMessage(Kopete::StatusMessage());
        }
    }
}

void TelepathyContact::onPublishStateChanged(Tp::Contact::PresenceState state)
{
    kDebug();

    Q_UNUSED(state);

    // TODO: Implement me!

}

void TelepathyContact::onBlockStatusChanged(bool blocked)
{
    kDebug() << blocked;

    // TODO: Implement me!

}

void TelepathyContact::onContactFetched(Tp::PendingOperation *pending)
{
    Tp::PendingContacts *contacts = qobject_cast<Tp::PendingContacts*>(pending);

    d->fetchingContact = false;

    if (contacts->isError() || contacts->contacts().isEmpty()) {
        kDebug() << "Getting contact failed:" << pending->errorName() << pending->errorMessage();
        emit internalContactFetched(false);
        return;
    }

    setInternalContact(contacts->contacts()[0]);
    emit internalContactFetched(true);
}

Tp::ContactPtr TelepathyContact::internalContact()
{
    return d->internalContact;
}

void TelepathyContact::deleteContact()
{
    kDebug();

    TelepathyAccount *tAccount = qobject_cast<TelepathyAccount*>(account());

    if (!tAccount) {
        kWarning() << "Account is not a TelepathyAccount.";
        return;
    }

    if (d->internalContact) {
        tAccount->deleteContact(d->internalContact);
    }
}

void TelepathyContact::requestAuthorization()
{
    kDebug();

    if (!internalContact()) {
        kDebug() << "No internal contact, can't request authorization.";
        return;
    }

    internalContact()->requestPresenceSubscription();
}

void TelepathyContact::sendAuthorization()
{
    kDebug();

    if (!internalContact()) {
        kDebug() << "No internal contact, can't send authorization.";
        return;
    }

    internalContact()->authorizePresencePublication();
}

void TelepathyContact::removeAuthorization()
{
    kDebug();

    if (!internalContact()) {
        kDebug() << "No internal contact, can't remove authorization.";
        return;
    }

    internalContact()->removePresencePublication();
}

void TelepathyContact::serverToLocalSync()
{
    kDebug() << "synchronizing groups: server -> local";

    /* Since this is a server->local sync we need to avoid
       sending it back to server */
    d->sync = false;

    if (d->internalContact->manager()->connection()->actualFeatures().contains(
        Tp::Connection::FeatureRosterGroups) &&
        !metaContact()->isTemporary()) {

        Kopete::GroupList groupsToRemoveFrom, groupsToAddTo;

        QStringList rosterGroups = d->internalContact->groups();

        foreach (Kopete::Group *kgroup, metaContact()->groups()) {
            if (!rosterGroups.contains(kgroup->displayName()))
                groupsToRemoveFrom.append(kgroup);
        }

        foreach (QString rgroup, rosterGroups) {
            bool found = false;
            foreach (Kopete::Group *kgroup, metaContact()->groups()) {
                if (kgroup->displayName() == rgroup) {
                    found = true;
                    break;
                }
            }

            if (!found)
                groupsToAddTo.append(
                        Kopete::ContactList::self()->findGroup(rgroup));
        }

        if ((groupsToAddTo.count() == 0) &&
                (groupsToRemoveFrom.contains(Kopete::Group::topLevel()))) {
            groupsToRemoveFrom.removeAll(Kopete::Group::topLevel());
        }

        foreach (Kopete::Group *group, groupsToRemoveFrom) {
            kDebug() << "Removing " << contactId() << " from group " <<
                group->displayName();
            metaContact()->removeFromGroup(group);
        }

        foreach (Kopete::Group *group, groupsToAddTo) {
            kDebug() << "Adding " << contactId() << " to group " <<
                group->displayName();
            metaContact()->addToGroup(group);
        }
    }

    /* Enable local->server sync again */
    d->sync = true;
}

#include "telepathycontact.moc"

