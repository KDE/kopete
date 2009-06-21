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

#ifndef TELEPATHYCONTACT_H_
#define TELEPATHYCONTACT_H_

#include <kopetecontact.h>
#include <kopetechatsession.h>

#include <QObject>
#include <QSharedPointer>

#include "telepathyaccount.h"

#include <TelepathyQt4/Contact>

namespace Kopete
{
    class MetaContact;
}

class TelepathyContact : public Kopete::Contact
{
    Q_OBJECT

public:
    TelepathyContact(TelepathyAccount *account, const QString &contactId, Kopete::MetaContact *parent);
    virtual ~TelepathyContact();

    virtual bool isReachable();
    virtual void serialize(QMap< QString, QString >& serializedData, QMap< QString, QString >& addressBookData);

    virtual QList<KAction *> *customContextMenuActions();
	virtual QList<KAction *> *customContextMenuActions( Kopete::ChatSession *manager );
    virtual Kopete::ChatSession *manager( CanCreateFlags canCreate = CannotCreate );

	void setInternalContact(QSharedPointer<Tp::Contact> contact);
	QSharedPointer<Tp::Contact> internalContact();

private slots:
	void onAliasChanged (const QString &);
	void onAvatarTokenChanged (const QString &);
	void onSimplePresenceChanged (const QString &, uint, const QString &);
	void onSubscriptionStateChanged (Tp::Contact::PresenceState);
	void onPublishStateChanged (Tp::Contact::PresenceState);
	void onBlockStatusChanged (bool);

private:
	class TelepathyContactPrivate;
	TelepathyContactPrivate * d;
};


#endif // TELEPATHYCONTACT_H_
