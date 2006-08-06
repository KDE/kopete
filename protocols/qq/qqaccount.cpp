/*
    qqaccount.cpp - Kopete QQ Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/


#include "qqaccount.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kactionmenu.h>
#include <kmenu.h>
#include <kicon.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetegroup.h"

#include "qqcontact.h"
#include "qqnotifysocket.h"
#include "qqfakeserver.h"
#include "qqprotocol.h"


QQAccount::QQAccount( QQProtocol *parent, const QString& accountID )
: Kopete::PasswordedAccount ( parent, accountID )
{
	m_notifySocket = 0L;
	m_connectstatus = QQProtocol::protocol()->Offline;
	m_newContactList=false;
 
	// Init the myself contact
	setMyself( new QQContact( this, accountId(), Kopete::ContactList::self()->myself() ) );
}

// The default implementation is TCP
QString QQAccount::serverName()
{
	return configGroup()->readEntry(  "serverName" , "tcpconn3.tencent.com" );
}

uint QQAccount::serverPort()
{
	return configGroup()->readEntry(  "serverPort" , 80 );
}

void QQAccount::connectWithPassword( const QString &password )
{
	kDebug ( 14210 ) << k_funcinfo << "connect with password" << password << endl;
	myself()->setOnlineStatus( QQProtocol::protocol()->qqOnline );
}

/* FIXME: move all things to connectWithPassword */
void QQAccount::connect( const Kopete::OnlineStatus& /* initialStatus */ )
{
	kDebug ( 14210 ) << k_funcinfo << endl;

	// FIXME: add invisible here!

	if ( isConnected() )
	{
		kDebug( 14210 ) << k_funcinfo <<"Ignoring Connect request "
			<< "(Already Connected)" << endl;
		return;
	}

	if ( m_notifySocket )
	{
		kDebug( 14210 ) << k_funcinfo <<"Ignoring Connect request (Already connecting)"  << endl;
		return;
	}
	/* Hard-coded password for debug only */
	m_password = "qqsucks";
	createNotificationServer(serverName(), serverPort());
}

void QQAccount::createNotificationServer( const QString &host, uint port )
{
	if(m_notifySocket) //we are switching from one to another notifysocket.
	{
		//remove every slots to that socket, so we won't delete receive signals
		// from the old socket thinking they are from the new one
		QObject::disconnect( m_notifySocket , 0, this, 0 );
		m_notifySocket->deleteLater(); //be sure it will be deleted
		m_notifySocket=0L;
	}

	myself()->setOnlineStatus( QQProtocol::protocol()->CNT );
	m_notifySocket = new QQNotifySocket( this, m_password );

	QObject::connect( m_notifySocket, SIGNAL( statusChanged( const Kopete::OnlineStatus & ) ),
		SLOT( slotStatusChanged( const Kopete::OnlineStatus & ) ) );
	QObject::connect( m_notifySocket, SIGNAL( newContactList() ),
		SLOT( slotNewContactList() ) );
	QObject::connect( m_notifySocket, SIGNAL( groupNames(const QStringList& )),
		SLOT( slotGroupNamesListed(const QStringList& ) ) );
	
	QObject::connect( m_notifySocket, SIGNAL( contactInGroup(int, char, int)),
		SLOT( slotContactInGroup(int, char, int)) );

	QObject::connect( m_notifySocket, SIGNAL( contactList(const Eva::ContactInfo &) ),
		SLOT( slotContactListed(const Eva::ContactInfo &) ) );

	QObject::connect( m_notifySocket, SIGNAL( contactStatusChanged(const Eva::ContactStatus&) ),
		SLOT( slotContactStatusChanged(const Eva::ContactStatus &) ) );
	m_notifySocket->connect(host, port);
}

void QQAccount::disconnect()
{
	if ( m_notifySocket )
		m_notifySocket->disconnect();
}

KActionMenu* QQAccount::actionMenu()
{
	KActionMenu *mActionMenu = Kopete::Account::actionMenu();

	mActionMenu->addSeparator();

	KAction *action;
	
	action = new KAction (KIcon("qq_showvideo"), i18n ("Show my own video..."), 0, "actionShowVideo");
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotShowVideo()) );
	mActionMenu->addAction(action);
	action->setEnabled( isConnected() );

	return mActionMenu;
}

QQNotifySocket *QQAccount::notifySocket()
{
	return m_notifySocket;
}



void QQAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason )
{
	if(status.status()== Kopete::OnlineStatus::Offline)
		disconnect();
	else if ( m_notifySocket )
	{
		// m_notifySocket->setStatus( status );
		//setPersonalMessage( reason );
	}
	else
	{
		kDebug( 14140 ) << k_funcinfo << "start connecting !!" << endl;
		m_connectstatus = status;
		connect( status );
	}
}


void QQAccount::setStatusMessage(const Kopete::StatusMessage& statusMessage)
{
	/* Not implemented in qq */
}


bool QQAccount::createContact(const QString& contactId, Kopete::MetaContact* parentContact)
{
	kDebug( 14140 ) << k_funcinfo << endl;
	QQContact* newContact = new QQContact( this, contactId, parentContact );
	return newContact != 0L;
}

void QQAccount::slotStatusChanged( const Kopete::OnlineStatus &status )
{
	myself()->setOnlineStatus( status );

	if(m_newContactList)
	{
		// m_newContactList = false;
		// Fetch the group names, later
		m_notifySocket->sendGetGroupNames();
		m_notifySocket->sendDownloadGroups();

		// Fetch the ContactList from the server.
		// m_notifySocket->sendContactList();
		// Fetch the relation of contact <--> group

	
	}
}

void QQAccount::slotGroupNamesListed(const QStringList& ql )
{
	kDebug ( 14210 ) << k_funcinfo << ql << endl;
	// Create the groups if necessary:
	QList<Kopete::Group*> groupList = Kopete::ContactList::self()->groups();
	Kopete::Group *g;
	Kopete::Group *fallback;

	// add the default group as #0 group.
	m_groupList += Kopete::Group::topLevel();
	
	for( QStringList::const_iterator it = ql.begin(); it != ql.end(); it++ )
	{
		foreach(g, groupList)
		{
			if( g->displayName() == *it )
				fallback = g;
			else
			{
				fallback = new Kopete::Group( *it );
				Kopete::ContactList::self()->addGroup( fallback );
			}
			m_groupList += fallback;
		}
	}
}

void QQAccount::slotShowVideo ()
{
	kDebug ( 14210 ) << k_funcinfo << endl;

	if (isConnected ())
		QQWebcamDialog *qqWebcamDialog = new QQWebcamDialog(0, 0);
	updateContactStatus();
}


void QQAccount::slotNewContactList()
{
	kDebug ( 14210 ) << k_funcinfo << endl;
	// remove the allow list.
	// TODO: cleanup QQAccount variables.
	KConfigGroup *config=configGroup();
	// config->writeEntry( "allowList" , QString() );

		// clear all date information which will be received.
		// if the information is not anymore on the server, it will not be received
		foreach ( Kopete::Contact *kc , contacts() )
		{
			QQContact *c = static_cast<QQContact *>( kc );
			c->setBlocked( false );
			c->setAllowed( false );
			c->setReversed( false );
			c->setDeleted( true );
			c->setInfo( "PHH", QString::null );
			c->setInfo( "PHW", QString::null );
			c->setInfo( "PHM", QString::null );
			// c->removeProperty( QQProtocol::protocol()->propGuid );
		}
		m_newContactList=true;
}

void QQAccount::slotContactInGroup(const int qqId, const char type, const int groupId )
{
	kDebug ( 14210 ) << k_funcinfo << endl;
	QString id = QString::number( qqId );
	QQContact *c = static_cast<QQContact *>( contacts()[ id ] );
	if( c )
		; // exited contact.
	else
	{
		Kopete::MetaContact *metaContact = new Kopete::MetaContact();
		c = new QQContact( this, id, metaContact );
		c->setOnlineStatus( QQProtocol::protocol()->Offline );
		Kopete::ContactList::self()->addMetaContact( metaContact );
		metaContact->addToGroup( m_groupList[groupId] );
	}
}
	
void QQAccount::slotContactListed( const Eva::ContactInfo& ci )
{
	// ignore also the myself contact.
	QString id = QString::number( ci.id );
	QString publicName = QString( QByteArray( ci.nick.data(), ci.nick.size() ) );

	if ( id == accountId() )
		return;

	QQContact *c = static_cast<QQContact *>( contacts()[ id ] );
	if( c )
		; // exited contact.
	else
	{
		Kopete::MetaContact *metaContact = new Kopete::MetaContact();
			
		c = new QQContact( this, id, metaContact );
		c->setOnlineStatus( QQProtocol::protocol()->Offline );
			
		if(!publicName.isEmpty() )
			c->setProperty( Kopete::Global::Properties::self()->nickName() , publicName );
		else
			c->removeProperty( Kopete::Global::Properties::self()->nickName() );
			
		Kopete::ContactList::self()->addMetaContact( metaContact );
	}

	return ;
}


void QQAccount::slotContactStatusChanged(const Eva::ContactStatus& cs)
{
	kDebug(14210) << k_funcinfo << "qqId = " << cs.qqId << " from " << cs.ip << ":" << cs.port << "status = " << cs.status << endl;

	QQContact* c = static_cast<QQContact*> (contacts()[ QString::number( cs.qqId ) ]);
	if (c) 
		c->setOnlineStatus( fromEvaStatus(cs.status) );
}


Kopete::OnlineStatus QQAccount::fromEvaStatus( char es )
{
	Kopete::OnlineStatus status;
	switch( es )
	{
		case Eva::Online : 
			status = Kopete::OnlineStatus( Kopete::OnlineStatus::Online );
			break;

		case Eva::Offline:
			status = Kopete::OnlineStatus( Kopete::OnlineStatus::Offline );
			break;

		case Eva::Away:
			status = Kopete::OnlineStatus( Kopete::OnlineStatus::Away );
			break;

		case Eva::Invisible:
			status = Kopete::OnlineStatus( Kopete::OnlineStatus::Invisible );
			break;
	}
	return status;
}

void QQAccount::updateContactStatus()
{
	QHashIterator<QString, Kopete::Contact*>itr( contacts() );
	for ( ; itr.hasNext(); ) {
		itr.next();
		itr.value()->setOnlineStatus( myself()->onlineStatus() );
	}
}

void QQAccount::receivedMessage( const QString &message )
{
	// Look up the contact the message is from
	QString from;
	QQContact* messageSender;

	from = message.section( ':', 0, 0 );
	Kopete::Contact* contact = contacts()[from];
	messageSender = dynamic_cast<QQContact *>( contact );

	kDebug( 14210 ) << k_funcinfo << " got a message from " << from << ", " << messageSender << ", is: " << message << endl;
	// Pass it on to the contact to process and display via a KMM
	if ( messageSender )
		;
		// messageSender->receivedMessage( message );
	else
		kWarning(14210) << k_funcinfo << "unable to look up contact for delivery" << endl;
}





#include "qqaccount.moc"
