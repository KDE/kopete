/*
 kopetedbusinterfaceprivate.h - Kopete D-Bus interface private class

 Copyright (c) 2008      by Dennis Nienhüser <earthwings@gentoo.org>
 
 Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

 *************************************************************************
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU Lesser General Public            *
 * License as published by the Free Software Foundation; either          *
 * version 2 of the License, or (at your option) any later version.      *
 *                                                                       *
 *************************************************************************
 */

#include "kopetedbusinterface_p.h"

#include <QtCore/QTimer>

#include <kopetecontactlist.h>
#include <kopetemetacontact.h>
#include <kopetecontact.h>
#include <kopetechatsessionmanager.h>
#include <kopeteviewmanager.h>
#include <kopetemessageevent.h>

ContactStalker::ContactStalker(Kopete::MetaContact *contact)
{
    m_contact = contact;
    QObject::connect( Kopete::ContactList::self(), 
    		SIGNAL(metaContactRemoved(Kopete::MetaContact*)),
    		this, SLOT(slotMetaContactRemoved(Kopete::MetaContact*)) );
    QObject::connect( contact, SIGNAL(onlineStatusChanged(Kopete::MetaContact*,Kopete::OnlineStatus::StatusType)),
            this, SLOT(slotEmitSignalDelayed()));
    QObject::connect( contact, SIGNAL(displayNameChanged(QString,QString)),
            this, SLOT(slotEmitSignalDelayed()));
    QObject::connect( contact, SIGNAL(photoChanged()),
            this, SLOT(slotEmitSignalDelayed()));
    QObject::connect( contact, SIGNAL(contactAdded(Kopete::Contact*)),
            this, SLOT(slotEmitSignalDelayed()));
    QObject::connect( contact, SIGNAL(contactRemoved(Kopete::Contact*)),
            this, SLOT(slotEmitSignalDelayed()));    
    
    QObject::connect(Kopete::ChatSessionManager::self(), 
    		SIGNAL(display(Kopete::Message&,Kopete::ChatSession*)), 
    		this, SLOT (messageAppended(Kopete::Message&,Kopete::ChatSession*)) );
    
    m_lastChange = QTime::currentTime();
    slotEmitSignal();
}

void ContactStalker::slotEmitSignalDelayed()
{
    const int timeout(1500);
    
    if (m_lastChange.elapsed() >= timeout)
    {
        m_lastChange = QTime::currentTime();   
        QTimer::singleShot(timeout, this, SLOT(slotEmitSignal()));
    }
}

void ContactStalker::slotEmitSignal()
{
	if (m_contact)
	{
		emit contactChanged(m_contact->metaContactId());
	}
}

void ContactStalker::messageAppended(Kopete::Message &message,
        Kopete::ChatSession *session)
{
    Q_UNUSED(session);
    if(!m_contact || !message.from())
    {
    	return;
    }

    if ( message.direction() == Kopete::Message::Inbound ) {
    	QString contactId = message.from()->metaContact()->metaContactId();
    	if (contactId == m_contact->metaContactId())
    	{
    		foreach(Kopete::Contact *subContact, m_contact->contacts())
    		{
    	        QList<Kopete::MessageEvent*> pendingMessages = KopeteViewManager::viewManager()->pendingMessages(subContact);
    	        foreach(Kopete::MessageEvent *event, pendingMessages)
    	        {
    	        	connect(event, SIGNAL(done(Kopete::MessageEvent*)), this, SLOT(slotEmitSignalDelayed()));
    	        }
    		}
    		
    		emit contactChanged(contactId);
    	}
    }
}

void ContactStalker::slotMetaContactRemoved(Kopete::MetaContact *contact)
{
	if (contact == m_contact)
	{
		m_contact = 0L;
		emit contactChanged(contact->metaContactId());
	}
}

KopeteDBusInterfacePrivate::KopeteDBusInterfacePrivate()
{
    QObject::connect(Kopete::ContactList::self(), 
    		SIGNAL(metaContactAdded(Kopete::MetaContact*)),
    		this, SLOT(slotMetaContactAdded(Kopete::MetaContact*)));	

    foreach( Kopete::MetaContact *contact, Kopete::ContactList::self()->metaContacts() )
    { 
        this->slotMetaContactAdded(contact);
    }
}

void KopeteDBusInterfacePrivate::slotMetaContactAdded(
        Kopete::MetaContact* contact)
{
    if ( contact ) {
        ContactStalker * stalker = new ContactStalker(contact);
        connect( stalker, SIGNAL(contactChanged(QString)), 
                this, SIGNAL(contactChanged(QString)));
    }
}

QStringList KopeteDBusInterfacePrivate::listContact(const QList<
        Kopete::MetaContact*> &contactList)
{
    QStringList result;

    foreach(Kopete::MetaContact *contact, contactList)
    { 
    	result << contact->metaContactId();
    }

return result;
}

Kopete::OnlineStatusManager::Categories KopeteDBusInterfacePrivate::status2Value(
        const QString &status)
{
    Kopete::OnlineStatusManager::Categories statusValue;
    if ( status.toLower() == QLatin1String("online") || status.toLower()
            == QLatin1String("available") ) {
        statusValue = Kopete::OnlineStatusManager::Online;
    } else if ( status.toLower() == QLatin1String("busy") ) {
        statusValue = Kopete::OnlineStatusManager::Busy;
    } else if ( status.toLower() == QLatin1String("away") ) {
        statusValue = Kopete::OnlineStatusManager::Away;
    } else if ( status.toLower() == QLatin1String("invisible") ) {
        statusValue = Kopete::OnlineStatusManager::Invisible;
    } else if ( status.toLower() == QLatin1String("offline") ) {
        statusValue = Kopete::OnlineStatusManager::Offline;
    }

    return statusValue;
}

Kopete::MetaContact *KopeteDBusInterfacePrivate::findContact(
        const QString &nameOrId)
{
    Kopete::MetaContact *contact = 0L;

    if ( nameOrId.count(':') == 2 ) {
        QStringList tokens = nameOrId.split(':');
        Q_ASSERT(tokens.size() == 3);
        Kopete::Contact *candidate = Kopete::ContactList::self()->findContact(
                tokens.at(0), tokens.at(1), tokens.at(2));
        if ( candidate ) {
            contact = candidate->metaContact();
        }
    }

    if ( !contact ) {
        contact = Kopete::ContactList::self()->metaContact(nameOrId);
    }

    if ( !contact ) {
        contact = Kopete::ContactList::self()->findMetaContactByContactId(
                nameOrId);
    }

    if ( !contact ) {
        contact = Kopete::ContactList::self()->findMetaContactByDisplayName(
                nameOrId);
    }

    return contact;
}

#include "kopetedbusinterface_p.moc"
