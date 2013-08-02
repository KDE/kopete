/*
    gwcontact.cpp - Kopete GroupWise Protocol

    Copyright (c) 2006      Novell, Inc	http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG http://www.suse.com
    
    Based on Testbed   
    Copyright (c) 2003      by Will Stephenson <will@stevello.free-online.co.uk>
    
	Blocking status taken from MSN
    Copyright (c) 2003      by Will Stephenson <will@stevello.free-online.co.uk>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Ryan Cumming <bodnar42@phalynx.dhs.org>
    Copyright (c) 2002-2003 by Martijn Klingens <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart <ogoffart@kde.org>
    
    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "gwcontact.h"
#include <qmap.h>

#include <kaction.h>
#include <KActionCollection>
#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>

#include <kopetemetacontact.h>
#include <kopeteuiglobal.h>
#include <kopeteprotocol.h>

#include "client.h"
#include "gwaccount.h"
#include "gwprotocol.h"
#include "privacymanager.h"
#include "userdetailsmanager.h"
#include "tasks/updatecontacttask.h"
#include "ui/gwcontactproperties.h"

using namespace GroupWise;

GroupWiseContact::GroupWiseContact( Kopete::Account* account, const QString &dn, 
			Kopete::MetaContact *parent, 
			const int objectId, const int parentId, const int sequence )
: Kopete::Contact( account, GroupWiseProtocol::dnToDotted( dn ), parent ), m_objectId( objectId ), m_parentId( parentId ),
  m_sequence( sequence ), m_actionBlock( 0 ), m_archiving( false ), m_deleting( false ), m_messageReceivedOffline( false )
{
	if ( dn.indexOf( '=' ) != -1 )
	{
		m_dn = dn;
	}
	connect( static_cast< GroupWiseAccount *>( account ), SIGNAL(privacyChanged(QString,bool)),
			SLOT(receivePrivacyChanged(QString,bool)) );
	setOnlineStatus( ( parent && parent->isTemporary() ) ? protocol()->groupwiseUnknown : protocol()->groupwiseOffline );
}

GroupWiseContact::~GroupWiseContact()
{
}

QString GroupWiseContact::dn() const
{
	return m_dn;
}

void GroupWiseContact::updateDetails( const ContactDetails & details )
{
	kDebug() ;
	if ( !details.cn.isNull() )
		setProperty( protocol()->propCN, QVariant(details.cn) );
	if ( !details.dn.isNull() )
		m_dn = details.dn;
	if ( !details.givenName.isNull() )
		setProperty( protocol()->propGivenName, QVariant(details.givenName) );
	if ( !details.surname.isNull() )
		setProperty( protocol()->propLastName, QVariant(details.surname) );
	if ( !details.fullName.isNull() )
		setProperty( protocol()->propFullName, QVariant(details.fullName) );
	m_archiving = details.archive;
	if ( !details.awayMessage.isNull() )
		setStatusMessage( details.awayMessage );
	
	m_serverProperties = details.properties;
	
	// work phone number
	if ( m_serverProperties.contains( "telephoneNumber" ) )
		setProperty( protocol()->propPhoneWork, QVariant(m_serverProperties.value( "telephoneNumber" )) );
	
	// mobile phone number
	if ( m_serverProperties.contains( "mobile" ) )
		setProperty( protocol()->propPhoneMobile, QVariant(m_serverProperties.value( "mobile" )) );
	
	// email
	if ( m_serverProperties.contains( "Internet EMail Address" ) )
		setProperty( protocol()->propEmail, QVariant(m_serverProperties.value( "Internet EMail Address" )) );
		
	if ( details.status != GroupWise::Invalid )
	{	
		Kopete::OnlineStatus status = protocol()->gwStatusToKOS( details.status );
		setOnlineStatus( status );
	}
}

GroupWiseProtocol *GroupWiseContact::protocol()
{
	return static_cast<GroupWiseProtocol *>( Kopete::Contact::protocol() );
}

GroupWiseAccount *GroupWiseContact::account()
{
	return static_cast<GroupWiseAccount *>( Kopete::Contact::account() );
}

bool GroupWiseContact::isReachable()
{
	// When we are invisible we can't start a chat with others, but we don't make isReachable return false, because then we
	// don't get any notification when we click on someone in the contact list.  Instead we warn the user when they try to send a message,
	// in GWChatSession
	// (This is a GroupWise rule, not a problem in Kopete)

	if ( account()->isConnected() && ( isOnline() || messageReceivedOffline() ) /* && account()->myself()->onlineStatus() != protocol()->groupwiseAppearOffline */)
		return true;
	if ( !account()->isConnected()/* || account()->myself()->onlineStatus() == protocol()->groupwiseAppearOffline*/ )
		return false;

	// fallback, something went wrong
	return false;
}

void GroupWiseContact::serialize( QMap< QString, QString > &serializedData, QMap< QString, QString > & /* addressBookData */ )
{
	serializedData[ "DN" ] = m_dn;
}

Kopete::ChatSession * GroupWiseContact::manager( Kopete::Contact::CanCreateFlags canCreate )
{
	//kDebug() << "called, canCreate: " << canCreate;

	Kopete::ContactPtrList chatMembers;
	chatMembers.append( this );

	return account()->chatSession( chatMembers, QString("")/*FIXME Check this is right*/, canCreate );
}

QList<KAction*> *GroupWiseContact::customContextMenuActions()
{
	QList<KAction *> * actions = new QList<KAction *>;

	// Block/unblock contact
	QString label = account()->isContactBlocked( m_dn ) ? i18n( "Unblock User" ) : i18n( "Block User" );
	if( !m_actionBlock )
	{
		m_actionBlock = new KAction( KIcon( "msn_blocked" ), label, 0 );
		QObject::connect( m_actionBlock, SIGNAL(triggered(bool)), SLOT(slotBlock()) );
	}
	else
		m_actionBlock->setText( label );
	m_actionBlock->setEnabled( account()->isConnected() );

	actions->append( m_actionBlock );
	// temporary action collection, used to apply Kiosk policy to the actions
	KActionCollection tempCollection((QObject*)0);
	tempCollection.addAction(QLatin1String("contactBlock"), m_actionBlock);

	return actions;
}

void GroupWiseContact::slotUserInfo()
{
	GroupWiseContactProperties *p = new GroupWiseContactProperties( this, Kopete::UI::Global::mainWidget() );
	p->setObjectName( "gwcontactproperties" );
}

QMap< QString, QVariant > GroupWiseContact::serverProperties()
{
	return m_serverProperties;
}

void GroupWiseContact::sendMessage( Kopete::Message &message )
{
	kDebug() ;
	manager()->appendMessage( message );
	// tell the manager it was sent successfully
	manager()->messageSucceeded();
}

void GroupWiseContact::deleteContact()
{
	account()->deleteContact( this );
}

void GroupWiseContact::sync( unsigned int)
{
	account()->syncContact( this );
}

void GroupWiseContact::slotBlock()
{
	kDebug() ;
	if ( account()->isConnected() )
	{
		if ( account()->isContactBlocked( m_dn ) )
			account()->client()->privacyManager()->setAllow( m_dn );
		else
			account()->client()->privacyManager()->setDeny( m_dn );
	}
}

void GroupWiseContact::receivePrivacyChanged( const QString & dn, bool allow )
{
	Q_UNUSED( allow );
	if ( dn == m_dn )	// set the online status back to itself. this will set the blocking state
		setOnlineStatus( this->onlineStatus() );
}

void GroupWiseContact::setOnlineStatus( const Kopete::OnlineStatus& status )
{
	setMessageReceivedOffline( false );
	if ( status == protocol()->groupwiseAwayIdle && status != onlineStatus() )
		setIdleTime( 1 );
	else if ( onlineStatus() == protocol()->groupwiseAwayIdle && status != onlineStatus() )
		setIdleTime( 0 );
	
	if ( account()->isContactBlocked( m_dn ) && status.internalStatus() < 15 )
	{
		Kopete::Contact::setOnlineStatus(Kopete::OnlineStatus(status.status(), (status.weight()==0) ? 0 : (status.weight() -1),
			protocol() , status.internalStatus()+15 , QStringList("msn_blocked"),
			i18n("%1|Blocked", status.description() ) ) );
	}
	else
	{
		if(status.internalStatus() >= 15)
		{	//the user is not blocked, but the status is blocked
			switch(status.internalStatus()-15)
			{
				case 0:
					Kopete::Contact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseUnknown );
					break;
				case 1:
					Kopete::Contact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseOffline );
					break;
				case 2:
					Kopete::Contact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseAvailable );
					break;
				case 3:
					Kopete::Contact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseBusy );
					break;
				case 4:
					Kopete::Contact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseAway );
					break;
				case 5:
					Kopete::Contact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseAwayIdle );
					break;
				default:
					Kopete::Contact::setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseUnknown );
					break;
			}
		}
		else
			Kopete::Contact::setOnlineStatus(status);
	}
}

bool GroupWiseContact::archiving() const
{
	return m_archiving;
}

bool GroupWiseContact::deleting() const
{
	return m_deleting;
}

void GroupWiseContact::setDeleting( bool deleting )
{
	m_deleting = deleting;
}

void GroupWiseContact::renamedOnServer()
{
	UpdateContactTask * uct = ( UpdateContactTask * )sender();
	if ( uct->success() )
	{
		setNickName( uct->displayName() );
	}
	else
		kDebug() << "rename failed, return code: " << uct->statusCode();
}

void GroupWiseContact::setMessageReceivedOffline( bool on )
{
	m_messageReceivedOffline = on;
}

bool GroupWiseContact::messageReceivedOffline() const
{
	return m_messageReceivedOffline;
}

#include "gwcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

