/*
 * telepathycontact.cpp - Telepathy Kopete Contact.
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
#include "telepathycontact.h"

// Qt includes
#include <QtCore/QPointer>
#include <QtGui/QImage>

// KDE includes
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// Kopete includes
#include <kopetechatsessionmanager.h>
#include <kopetemetacontact.h>
#include <kopeteuiglobal.h>
#include <kopeteavatarmanager.h>

// QtTapioca includes
#include <QtTapioca/Contact>
#include <QtTapioca/TextChannel>
#include <QtTapioca/Avatar>

// Telepathy includes
#include "telepathyaccount.h"
#include "telepathyprotocol.h"
#include "telepathycontactmanager.h"
#include "telepathychatsession.h"

using namespace QtTapioca;

class TelepathyContact::Private
{
public:
	Private()
	{}

	QPointer<QtTapioca::Contact> internalContact;
	QPointer<Kopete::ChatSession> currentChatSession;
};

TelepathyContact::TelepathyContact(TelepathyAccount *account, const QString &contactId, Kopete::MetaContact *parent)
 : Kopete::Contact(account, contactId, parent), d(new Private)
{
	setOnlineStatus( TelepathyProtocol::protocol()->Offline );
}

TelepathyContact::~TelepathyContact()
{
	delete d;
}

TelepathyAccount *TelepathyContact::account()
{
	return static_cast<TelepathyAccount*>( Kopete::Contact::account() );
}

QtTapioca::Contact *TelepathyContact::internalContact()
{
// 	Q_ASSERT( !d->internalContact.isNull() );
	return d->internalContact;
}

void TelepathyContact::setInternalContact(QtTapioca::Contact *internalContact)
{
	kDebug(TELEPATHY_DEBUG_AREA) << "Updating internal contact pointer for " << contactId();

	if( !d->internalContact.isNull() )
	{
		// Disconnect signals from previous internal contact
		d->internalContact->disconnect();
	}
	d->internalContact = internalContact;

	// Connect signal/slots
	connect(d->internalContact, SIGNAL(presenceUpdated(QtTapioca::ContactBase*, QtTapioca::ContactBase::Presence, QString)), this, SLOT(telepathyPresenceUpdated(QtTapioca::ContactBase*, QtTapioca::ContactBase::Presence, QString)));
	connect(d->internalContact, SIGNAL(aliasChanged(QtTapioca::ContactBase*,QString)), this, SLOT(telepathyAliasChanged(QtTapioca::ContactBase*,QString)));
	connect(d->internalContact, SIGNAL(avatarUpdated(QtTapioca::ContactBase*,QString)), this, SLOT(telepathyAvatarChanged(QtTapioca::ContactBase*,QString)));
	connect(d->internalContact, SIGNAL(avatarReceived(QtTapioca::ContactBase*,QtTapioca::Avatar*)), this, SLOT(telepathyAvatarReceived(QtTapioca::ContactBase*,QtTapioca::Avatar*)));
	// Set initial presence
	// \todo: FIXME
	//TelepathyProtocol::protocol()->telepathyStatusToKopete( d->internalContact->presence() );

	// Set nickname/alias
	setNickName( d->internalContact->alias() );

	// Request avatar
	d->internalContact->requestAvatar();
}

bool TelepathyContact::isReachable()
{
	return account()->isConnected();
}

void TelepathyContact::serialize(QMap< QString, QString >& serializedData, QMap< QString, QString >& addressBookData)
{
	Q_UNUSED(serializedData);
	Q_UNUSED(addressBookData);
	// Nothing specific to serialize yet.
}

QList<KAction *> *TelepathyContact::customContextMenuActions()
{
	// TODO: Optimize
	QList<KAction*> *actionList = new QList<KAction*>;

	KAction *actionAuthorize = new KAction( KIcon("mail-forward"), i18n("Authorize Contact"), 0 );
	connect( actionAuthorize, SIGNAL(triggered(bool)), this, SLOT(actionAuthorize()) );
	actionAuthorize->setEnabled(false);
	if( internalContact() && internalContact()->authorizationStatus() != QtTapioca::Contact::Authorized )
		actionAuthorize->setEnabled(true);

	KAction *actionSubscribe = new KAction( KIcon("mail-reply-sender"), i18n("Subscribe to Contact"), 0 );
	connect( actionSubscribe, SIGNAL(triggered(bool)), this, SLOT(actionSubscribe()) );
	actionSubscribe->setEnabled(false);
	if( internalContact() && internalContact()->subscriptionStatus() != QtTapioca::Contact::Subscribed )
		actionSubscribe->setEnabled(true);

	actionList->append( actionAuthorize );
	actionList->append( actionSubscribe );

	return actionList;
}

Kopete::ChatSession *TelepathyContact::manager(CanCreateFlags canCreate)
{
	if( d->currentChatSession.isNull() )
	{
		QList<Kopete::Contact*> others;
		others.append( this );

		// Fist try to find an existing chat session
		Kopete::ChatSession *existingSession = Kopete::ChatSessionManager::self()->findChatSession( account()->myself(), others, account()->protocol() );
		if( existingSession )
		{
			d->currentChatSession = existingSession;
		}
		// Else create a new chat session and text channel
		else if( canCreate == Kopete::Contact::CanCreate )
		{
			TelepathyChatSession *newSession = new TelepathyChatSession( account()->myself(), others, account()->protocol() );
			// Assume that we create a new session
			TextChannel *textChannel = account()->createTextChannel( internalContact() );
			if( textChannel )
			{
				newSession->setTextChannel(textChannel);
				d->currentChatSession = newSession;
			}
		}
	}

	return d->currentChatSession;
}

void TelepathyContact::deleteContact()
{
	if( !account()->isConnected() )
	{
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error, i18n("You must be connected to delete a contact."), i18n("Telepathy plugin") );
		return;
	}

	account()->contactManager()->removeContact(this);
}

void TelepathyContact::telepathyPresenceUpdated(Telepathy::Client::Account *account, const QString &presenceMessage)
{
	Telepathy::SimplePresence presence = account->currentPresence();
	Kopete::OnlineStatus newStatus = TelepathyProtocol::protocol()->telepathyStatusToKopete(presence.type);

	kDebug(TELEPATHY_DEBUG_AREA) << "Updating " << contactId() << " presence to " << newStatus.description();
	kDebug(TELEPATHY_DEBUG_AREA) << "New Status Message for " << contactId() << ": " << presenceMessage;

	setOnlineStatus( newStatus );
	setStatusMessage( Kopete::StatusMessage(presenceMessage) );
}

void TelepathyContact::telepathyAliasChanged(QtTapioca::ContactBase *contactBase, const QString &newAlias)
{
	Q_UNUSED(contactBase);

	kDebug(TELEPATHY_DEBUG_AREA) << "Changing " << contactId() << " alias to " << newAlias;

	setNickName( newAlias );
}

void TelepathyContact::telepathyAvatarChanged(QtTapioca::ContactBase *contactBase, const QString &newToken)
{
	QString currentToken = property(TelepathyProtocol::protocol()->propAvatarToken).value().toString();
	if( currentToken != newToken )
	{
		internalContact()->requestAvatar();
	}
}

void TelepathyContact::telepathyAvatarReceived(QtTapioca::ContactBase *contactBase, QtTapioca::Avatar *avatar)
{
	kDebug(TELEPATHY_DEBUG_AREA) << "Received avatar for " << contactId();

	// Remove the avatar if the data is empty and exit the method
	if( avatar->data().isEmpty() )
	{
		kDebug(TELEPATHY_DEBUG_AREA) << "WARNING: Avatar image is empty. Removing the avatar";

		removeProperty( Kopete::Global::Properties::self()->photo() );
		removeProperty( TelepathyProtocol::protocol()->propAvatarToken );

		return;
	}

	// Guess file format from header for now
	QImage avatarImage = QImage::fromData( avatar->data() );

	// Create/Update avatar entry for this contact
	Kopete::AvatarManager::AvatarEntry newAvatar;
	newAvatar.name = contactId();
	newAvatar.contact = this;
	newAvatar.image = avatarImage;
	newAvatar.category = Kopete::AvatarManager::Contact;

	Kopete::AvatarManager::AvatarEntry result = Kopete::AvatarManager::self()->add( newAvatar );
	
	if( !result.path.isEmpty() )
	{
		kDebug(TELEPATHY_DEBUG_AREA) << "Setting avatar information for " << contactId();

		// Set avatar in Kopete
		setProperty( Kopete::Global::Properties::self()->photo(), result.path );
		setProperty( TelepathyProtocol::protocol()->propAvatarToken, avatar->token() );
	}
	else
	{
		kDebug(TELEPATHY_DEBUG_AREA) << "Removing avatar information for " << contactId();

		removeProperty( Kopete::Global::Properties::self()->photo() );
		removeProperty( TelepathyProtocol::protocol()->propAvatarToken );
	}
}

void TelepathyContact::actionAuthorize()
{
	internalContact()->authorize(true);
}

void TelepathyContact::actionSubscribe()
{
	internalContact()->subscribe(true);
}

#include "telepathycontact.moc"
