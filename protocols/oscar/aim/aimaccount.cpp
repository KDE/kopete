/*
  aimaccount.cpp  -  Oscar Protocol Plugin, AIM part

  Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include <qdom.h>
#include <qfile.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <kmdcodec.h>

#include "kopeteawayaction.h"
#include "kopetepassword.h"
#include "kopetestdaction.h"
#include "kopeteuiglobal.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include <kopeteuiglobal.h>

#include "aimprotocol.h"
#include "aimaccount.h"
#include "aimcontact.h"
#include "aimuserinfo.h"
#include "aimjoinchat.h"
#include "oscarmyselfcontact.h"

#include "oscarutils.h"
#include "client.h"
#include "ssimanager.h"


const DWORD AIM_ONLINE = 0x0;
const DWORD AIM_AWAY = 0x1;

namespace Kopete { class MetaContact; }

AIMMyselfContact::AIMMyselfContact( AIMAccount *acct )
: OscarMyselfContact( acct )
{
	m_acct = acct;
}

void AIMMyselfContact::userInfoUpdated()
{
	if ( ( details().userClass() & 32 ) == 0 )
		setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusOnline ); //we're online
	else
		setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusAway ); //we're away
}

void AIMMyselfContact::setOwnProfile( const QString& newProfile )
{
	m_profileString = newProfile;
	if ( m_acct->isConnected() )
		m_acct->engine()->updateProfile( newProfile );
}

QString AIMMyselfContact::userProfile()
{
	return m_profileString;
}


AIMAccount::AIMAccount(Kopete::Protocol *parent, QString accountID, const char *name)
	: OscarAccount(parent, accountID, name, false)
{
	kdDebug(14152) << k_funcinfo << accountID << ": Called."<< endl;
	AIMMyselfContact* mc = new AIMMyselfContact( this );
	setMyself( mc );
	myself()->setOnlineStatus( static_cast<AIMProtocol*>( parent )->statusOffline );
	QString profile = configGroup()->readEntry( "Profile",
		i18n( "Visit the Kopete website at <a href=\"http://kopete.kde.org\">http://kopete.kde.org</a>") );
	mc->setOwnProfile( profile );

    m_joinChatDialog = 0;
	QObject::connect( Kopete::ContactList::self(),
	                  SIGNAL( globalIdentityChanged( const QString&, const QVariant& ) ),
	                  this,
	                  SLOT( globalIdentityChanged( const QString&, const QVariant& ) ) );

	QObject::connect( engine(), SIGNAL( iconNeedsUploading() ), this,
	                  SLOT( sendBuddyIcon() ) );
}

AIMAccount::~AIMAccount()
{
}

OscarContact *AIMAccount::createNewContact( const QString &contactId, Kopete::MetaContact *parentContact, const SSI& ssiItem )
{
	AIMContact* contact = new AIMContact( this, contactId, parentContact, QString::null, ssiItem );
	if ( !ssiItem.alias().isEmpty() )
		contact->setProperty( Kopete::Global::Properties::self()->nickName(), ssiItem.alias() );

	return contact;
}

QString AIMAccount::sanitizedMessage( const Oscar::Message& message )
{
	QDomDocument doc;
	QString domError;
	int errLine = 0, errCol = 0;
	doc.setContent( message.text(), false, &domError, &errLine, &errCol );
	if ( !domError.isEmpty() ) //error parsing, do nothing
	{
		kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "error from dom document conversion: "
			<< domError << endl;
		return message.text();
	}
	else
	{
		kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "conversion to dom document successful."
			<< "looking for font tags" << endl;
		QDomNodeList fontTagList = doc.elementsByTagName( "font" );
		if ( fontTagList.count() == 0 )
		{
			kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "No font tags found. Returning normal message" << endl;
			return message.text();
		}
		else
		{
			kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "Found font tags. Attempting replacement" << endl;
			uint numFontTags = fontTagList.count();
			for ( uint i = 0; i < numFontTags; i++ )
			{
				QDomNode fontNode = fontTagList.item(i);
				QDomElement fontEl;
				if ( !fontNode.isNull() && fontNode.isElement() )
					fontEl = fontTagList.item(i).toElement();
				else
					continue;
				if ( fontEl.hasAttribute( "back" ) )
				{
					kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Found attribute to replace. Doing replacement" << endl;
					QString backgroundColor = fontEl.attribute( "back" );
					backgroundColor.insert( 0, "background-color: " );
					backgroundColor.append( ';' );
					fontEl.setAttribute( "style", backgroundColor );
					fontEl.removeAttribute( "back" );
				}
			}
		}
	}
	kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "sanitized message is " << doc.toString();
	return doc.toString();
}

KActionMenu* AIMAccount::actionMenu()
{
//	kdDebug(14152) << k_funcinfo << accountId() << ": Called." << endl;
	// mActionMenu is managed by Kopete::Account.  It is deleted when
	// it is no longer shown, so we can (safely) just make a new one here.
	KActionMenu *mActionMenu = new KActionMenu(accountId(),
		myself()->onlineStatus().iconFor( this ), this, "AIMAccount::mActionMenu");

	AIMProtocol *p = AIMProtocol::protocol();

	QString accountNick = myself()->property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	mActionMenu->popupMenu()->insertTitle( myself()->onlineStatus().iconFor( myself() ),
			i18n( "%2 <%1>" ).arg( accountId(), accountNick ));

	mActionMenu->insert( new KAction( i18n("Online"), p->statusOnline.iconFor( this ), 0, this,
	             SLOT( slotGoOnline() ), mActionMenu, "AIMAccount::mActionOnline") );

	KAction* mActionAway = new Kopete::AwayAction(i18n("Away"), p->statusAway.iconFor( this ), 0, this,
	             SLOT(slotGoAway( const QString & )), this, "AIMAccount::mActionNA" );
	mActionAway->setEnabled( isConnected() );
	mActionMenu->insert( mActionAway );

	KAction* mActionOffline = new KAction( i18n("Offline"), p->statusOffline.iconFor(this), 0, this,
	             SLOT( slotGoOffline() ), mActionMenu, "AIMAccount::mActionOffline");

	mActionMenu->insert( mActionOffline );
	mActionMenu->popupMenu()->insertSeparator();

    KAction* m_joinChatAction = new KAction( i18n( "Join a chat..." ), QString::null,  0,  this,
                                             SLOT( slotJoinChat() ), mActionMenu, "join_a_chat" );
    mActionMenu->insert( m_joinChatAction );
    mActionMenu->insert( KopeteStdAction::contactInfo( this, SLOT( slotEditInfo() ), mActionMenu, "AIMAccount::mActionEditInfo" ) );

	return mActionMenu;
}

void AIMAccount::setAway(bool away, const QString &awayReason)
{
// 	kdDebug(14152) << k_funcinfo << accountId() << "reason is " << awayReason << endl;
	if ( away )
	{
		engine()->setStatus( Client::Away, awayReason );
		AIMMyselfContact* me = static_cast<AIMMyselfContact *> ( myself() );
		me->setLastAwayMessage(awayReason);
		me->setProperty( Kopete::Global::Properties::self()->awayMessage(), awayReason );
	}
	else
	{
		engine()->setStatus( Client::Online );
		AIMMyselfContact* me = static_cast<AIMMyselfContact *> ( myself() );
		me->setLastAwayMessage(QString::null);
		me->removeProperty( Kopete::Global::Properties::self()->awayMessage() );
	}
}

void AIMAccount::setUserProfile(const QString &profile)
{
	kdDebug(14152) << k_funcinfo << "called." << endl;
	AIMMyselfContact* aimmc = dynamic_cast<AIMMyselfContact*>( myself() );
	if ( aimmc )
		aimmc->setOwnProfile( profile );
	configGroup()->writeEntry( QString::fromLatin1( "Profile" ), profile );
}

void AIMAccount::slotEditInfo()
{
	AIMUserInfoDialog *myInfo = new AIMUserInfoDialog(static_cast<AIMContact *>( myself() ), this, true, 0L, "myInfo");
	myInfo->exec(); // This is a modal dialog
}

void AIMAccount::globalIdentityChanged( const QString& key, const QVariant& value )
{
	//do something with the photo
	kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "Global identity changed" << endl;
	kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "key: " << key << endl;
	kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "value: " << value << endl;
	if ( key == Kopete::Global::Properties::self()->nickName().key() )
	{
		//edit ssi item to change alias (if possible)
	}

	if ( key == Kopete::Global::Properties::self()->photo().key() )
	{
		//yay for brain damage. since i have no way to access the global
		//properties outside of this slot, i've got to set them somewhere
		//else so i can get at them later.
		myself()->setProperty( Kopete::Global::Properties::self()->photo(), value.toString() );

		//generate a new icon hash
		//photo is a url, gotta load it first.
		QFile iconFile( value.toString() );
		iconFile.open( IO_ReadOnly );
		kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "hashing global identity image" << endl;
		KMD5 iconHash;
		iconHash.update( iconFile );
		kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo  << "hash is :" << iconHash.hexDigest() << endl;
		//find old item, create updated item
		if ( engine()->isActive() )
		{
			SSIManager* ssi = engine()->ssiManager();
			Oscar::SSI item = ssi->findItemForIconByRef( 1 );

			if ( !item )
			{
				kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "no existing icon hash item in ssi. creating new"
					<< endl;
				TLV t;
				t.type = 0x00D5;
				t.data.resize( 18 );
				t.data[0] = 0x00;
				t.data[1] = 0x10;
				memcpy(t.data.data() + 2, iconHash.rawDigest(), 16);
				t.length = t.data.size();

				QValueList<Oscar::TLV> list;
				list.append( t );

				Oscar::SSI s( "1", 0, ssi->nextContactId(), ROSTER_BUDDYICONS, list );

				//item is a non-valid ssi item, so the function will add an item
				kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "setting new icon item" << endl;
				engine()->modifySSIItem( item, s );
			}
			else
			{ //found an item
				Oscar::SSI s(item);
				kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "modifying old item in ssi."
					<< endl;
				QValueList<TLV> tList( item.tlvList() );
				TLV t = Oscar::findTLV( tList, 0x00D5 );
				if ( !t )
					return;
				tList.remove( t );
				t.data.resize( 18 );
				t.data[0] = 0x00;
				t.data[1] = 0x10;
				memcpy(t.data.data() + 2, iconHash.rawDigest(), 16);
				t.length = t.data.size();
				tList.append( t );
				item.setTLVList( tList );
				//s is old, item is new. modification will occur
				engine()->modifySSIItem( s, item );
			}
		}
		iconFile.close();
	}
}


void AIMAccount::sendBuddyIcon()
{
	QString photoPath = myself()->property( Kopete::Global::Properties::self()->photo() ).value().toString();
	kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << photoPath << endl;
	QFile iconFile( photoPath );
	iconFile.open( IO_ReadOnly );
	QByteArray imageData = iconFile.readAll();
	engine()->sendBuddyIcon( imageData );
}

void AIMAccount::slotJoinChat()
{
    //get the exchange info
    //create the dialog
    //join the chat room
    if ( !m_joinChatDialog )
    {
        m_joinChatDialog = new AIMJoinChatUI( this, false, Kopete::UI::Global::mainWidget() );
        m_joinChatDialog->show();
    }
    else
        m_joinChatDialog->raise();
}

void AIMAccount::slotGoOnline()
{
	if ( myself()->onlineStatus().status() == Kopete::OnlineStatus::Away )
	{
		kdDebug(14152) << k_funcinfo << accountId() << " was away. welcome back." << endl;
		engine()->setStatus( Client::Online );
		myself()->removeProperty( Kopete::Global::Properties::self()->awayMessage() );
	}
	else if ( myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline )
	{
		kdDebug(14152) << k_funcinfo << accountId() << " was offline. time to connect" << endl;
		OscarAccount::connect();
	}
	else
	{
		kdDebug(14152) << k_funcinfo << accountId() << " is already online, doing nothing" << endl;
	}
}

void AIMAccount::slotGoAway(const QString &message)
{
	kdDebug(14152) << k_funcinfo << message << endl;
	setAway(true, message);
}

void AIMAccount::joinChatDialogClosed()
{
    m_joinChatDialog->delayedDestruct();
    m_joinChatDialog = 0L;
}

void AIMAccount::disconnected( DisconnectReason reason )
{
	kdDebug( OSCAR_AIM_DEBUG ) << k_funcinfo << "Attempting to set status offline" << endl;
	myself()->setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusOffline );
	OscarAccount::disconnected( reason );
}

void AIMAccount::messageReceived( const Oscar::Message& message )
{
	kdDebug(14152) << k_funcinfo << " Got a message, calling OscarAccount::messageReceived" << endl;
	// Want to call the parent to do everything else
	OscarAccount::messageReceived(message);

	// Check to see if our status is away, and send an away message
	// Might be duplicate code from the parent class to get some needed information
	// Perhaps a refactoring is needed.
	kdDebug(14152) << k_funcinfo << "Checking to see if I'm online.." << endl;
	if( myself()->onlineStatus().status() == Kopete::OnlineStatus::Away )
	{
		QString sender = Oscar::normalize( message.sender() );
		AIMContact* aimSender = static_cast<AIMContact *> ( contacts()[sender] ); //should exist now
		if ( !aimSender )
		{
			kdWarning(OSCAR_RAW_DEBUG) << "For some reason, could not get the contact "
				<< "That this message is from: " << message.sender() << ", Discarding message" << endl;
			return;
		}
		// Create, or get, a chat session with the contact
		Kopete::ChatSession* chatSession = aimSender->manager( Kopete::Contact::CanCreate );

		// get the away message we have set
		AIMMyselfContact* myContact = static_cast<AIMMyselfContact *> ( myself() );
		QString msg = myContact->lastAwayMessage();
		kdDebug(14152) << k_funcinfo << "Got away message: " << msg << endl;
		// Create the message
		Kopete::Message chatMessage( myself(), aimSender, msg, Kopete::Message::Outbound,
		                             Kopete::Message::RichText );
		kdDebug(14152) << k_funcinfo << "Sending autoresponse" << endl;
		// Send the message
		aimSender->sendAutoResponse( chatMessage );
	}
}

void AIMAccount::connectWithPassword( const QString & )
{
	kdDebug(14152) << k_funcinfo << "accountId='" << accountId() << "'" << endl;

	// Get the screen name for this account
	QString screenName = accountId();
	QString server = configGroup()->readEntry( "Server", QString::fromLatin1( "login.oscar.aol.com" ) );
	uint port = configGroup()->readNumEntry( "Port", 5190 );

	Connection* c = setupConnection( server, port );

	QString _password = password().cachedValue();
	if ( _password.isEmpty() )
	{
		kdDebug(14150) << "Kopete is unable to attempt to sign-on to the "
			<< "AIM network because no password was specified in the "
			<< "preferences." << endl;
	}
	else if ( myself()->onlineStatus() == static_cast<AIMProtocol*>( protocol() )->statusOffline )
	{
		kdDebug(14152) << k_funcinfo << "Logging in as " << accountId() << endl ;
		engine()->start( server, port, accountId(), _password );
		engine()->connectToServer( c, server, true /* doAuth */ );
		myself()->setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusConnecting );
	}
}

#include "aimaccount.moc"
//kate: tab-width 4; indent-mode csands;
