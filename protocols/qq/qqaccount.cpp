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

	QObject::connect( m_notifySocket, SIGNAL( statusChanged( const Kopete::OnlineStatus & ) ),
		SLOT( slotStatusChanged( const Kopete::OnlineStatus & ) ) );
	QObject::connect( m_notifySocket, SIGNAL( newContactList() ),
		SLOT( slotNewContactList() ) );

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

	mActionMenu->kMenu()->insertSeparator();

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
	QQContact* newContact = new QQContact( this, contactId, parentContact );
	return newContact != 0L;
}

void QQAccount::slotStatusChanged( const Kopete::OnlineStatus &status )
{
//	kDebug( 14140 ) << k_funcinfo  << status.internalStatus() <<  endl;
	myself()->setOnlineStatus( status );

	if(m_newContactList)
	{
		m_newContactList=false;

		QHash<QString,Kopete::Contact*> contactList = contacts();
		QHash<QString,Kopete::Contact*>::Iterator it, itEnd = contactList.end();
		for ( it = contactList.begin(); it != itEnd; ++it )
		{
			QQContact *c = static_cast<QQContact *>( *it );
			if(c && c->isDeleted() && c->metaContact() && !c->metaContact()->isTemporary() && c!=myself())
			{
				if(c->serverGroups().isEmpty())
				{ //the contact is new, add it on the server
					c->setOnlineStatus( QQProtocol::protocol()->Offline );
					// addContactServerside( c->contactId() , c->metaContact()->groups() );
				}
				else //the contact had been deleted, give him the unknown status
				{
					c->clearServerGroups();
					c->setOnlineStatus( QQProtocol::protocol()->UNK );
				}
			}
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
		m_oldGroupList=m_groupList;
		QMap<QString, Kopete::Group*>::Iterator it, itEnd = m_oldGroupList.end();
		for( it=m_oldGroupList.begin() ; it != itEnd ; ++it )
		{	//they are about to be changed
			if(it.value())
				;
				// it.value()->setPluginData( protocol(), accountId() + " id", QString::null );
		}

		m_allowList.clear();
		m_blockList.clear();
		m_reverseList.clear();
		m_groupList.clear();
		KConfigGroup *config=configGroup();
		config->writeEntry( "blockList" , QString() ) ;
		config->writeEntry( "allowList" , QString() );
		config->writeEntry( "reverseList" , QString() );

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

void QQAccount::slotContactListed( const QString& handle, const QString& publicName, const QString &contactGuid, uint lists, const QString& groups )
{
	// On empty lists handle might be empty, ignore that
	// ignore also the myself contact.
	if ( handle.isEmpty() || handle==accountId())
		return;

	QQContact *c = static_cast<QQContact *>( contacts()[ handle ] );
	return ;
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
