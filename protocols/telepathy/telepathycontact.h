/*
 * telepathycontact.h - Telepathy Kopete Contact.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#ifndef TELEPATHYCONTACT_H
#define TELEPATHYCONTACT_H

// Qt Includes
#include <QtCore/QMap>
#include <QtCore/QList>

// Kopete includes
#include <kopetecontact.h>

// QtTapioca includes
#include <QtTapioca/ContactInfo>

class KAction;

namespace QtTapioca
{
	class Contact;
}

namespace Kopete
{
	class ChatSession;
	class MetaContact;
}
class TelepathyAccount;

using namespace QtTapioca;

class TelepathyContact : public Kopete::Contact
{
	Q_OBJECT
public:
	TelepathyContact(TelepathyAccount *account, const QString &contactId, Kopete::MetaContact *parent);
	~TelepathyContact();

	virtual bool isReachable();
	virtual void serialize(QMap< QString, QString >& serializedData, QMap< QString, QString >& addressBookData);
	
	virtual QList<KAction *> *customContextMenuActions();
	virtual Kopete::ChatSession *manager( CanCreateFlags canCreate = CannotCreate );

	void setInternalContact(QtTapioca::Contact *internalContact);

private slots:
	void slotPresenceUpdated(ContactInfo *contactInfo, ContactInfo::Presence presence, const QString &presenceMessage);

private:
	QtTapioca::Contact *internalContact();

private:
	class Private;
	Private *d;
};
#endif
