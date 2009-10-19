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

#ifndef LIB_KOPETE_TELEPATHY_TELEPATHYCONTACT_H
#define LIB_KOPETE_TELEPATHY_TELEPATHYCONTACT_H

#include <KopeteTelepathy/telepathyaccount.h>

#include <kopetecontact.h>
#include <kopetechatsession.h>

#include <TelepathyQt4/Contact>

namespace Kopete
{
class MetaContact;
}

class KOPETE_EXPORT TelepathyContact : public Kopete::Contact
{
    Q_OBJECT

public:
    TelepathyContact(TelepathyAccount *account, const QString &contactId,
                     Kopete::MetaContact *parent);
    virtual ~TelepathyContact();

    virtual bool isReachable();
    virtual void serialize(QMap< QString, QString >& serializedData,
                           QMap< QString, QString >& addressBookData);

    virtual QList<KAction *> *customContextMenuActions();
    virtual QList<KAction *> *customContextMenuActions(Kopete::ChatSession *manager);
    virtual Kopete::ChatSession *manager(CanCreateFlags canCreate = CannotCreate);
    virtual Kopete::ChatSession *manager(Kopete::ContactPtrList members,
                                         CanCreateFlags canCreate = CannotCreate);
    virtual void sync(unsigned int flags);
    virtual void sendFile(const KUrl &sourceURL = KUrl(),
                          const QString &fileName = QString(),
                          uint fileSize = 0L);

    void setInternalContact(Tp::ContactPtr contact);
    Tp::ContactPtr internalContact();

    QString storedAvatarToken() const;
    QString storedAvatarPath() const;

public Q_SLOTS:
    virtual void deleteContact();

private slots:
    void onAliasChanged(const QString &);
    void onAvatarTokenChanged(const QString &);
    void onAvatarRetrieved(uint, const QString&, const QByteArray&, const QString&);
    void onSimplePresenceChanged(const QString &, uint, const QString &);
    void onSubscriptionStateChanged(Tp::Contact::PresenceState);
    void onPublishStateChanged(Tp::Contact::PresenceState);
    void onBlockStatusChanged(bool);

private:
    void serverToLocalSync();
    class TelepathyContactPrivate;
    TelepathyContactPrivate * d;
};


#endif // Header guard

