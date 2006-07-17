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
#include <kdialog.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kcodecs.h>
#include <ktoggleaction.h>

#include "kopeteawayaction.h"
#include "kopetepassword.h"
#include "kopetestdaction.h"
#include "kopeteuiglobal.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopetechatsessionmanager.h"
#include "kopeteview.h"
#include <kopeteuiglobal.h>

#include "aimprotocol.h"
#include "aimaccount.h"
#include "aimchatsession.h"
#include "aimcontact.h"
#include "aimuserinfo.h"
#include "aimjoinchat.h"
#include "oscarmyselfcontact.h"
#include "oscarvisibilitydialog.h"

#include "oscarutils.h"
#include "client.h"
#include "contactmanager.h"


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

Kopete::ChatSession* AIMMyselfContact::manager( Kopete::Contact::CanCreateFlags canCreate,
		Oscar::WORD exchange, const QString& room )
{
	kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << endl;
	Kopete::ContactPtrList chatMembers;
	chatMembers.append( this );
	Kopete::ChatSession* genericManager = 0L;
	genericManager = Kopete::ChatSessionManager::self()->findChatSession( account()->myself(), chatMembers, protocol() );
	AIMChatSession* session = dynamic_cast<AIMChatSession*>( genericManager );

	if ( !session && canCreate == Contact::CanCreate )
	{
		session = new AIMChatSession( this, chatMembers, account()->protocol(), exchange, room );
		session->setEngine( m_acct->engine() );

		connect( session, SIGNAL( messageSent( Kopete::Message&, Kopete::ChatSession* ) ),
				this, SLOT( sendMessage( Kopete::Message&, Kopete::ChatSession* ) ) );
		m_chatRoomSessions.append( session );
	}
	return session;
}

void AIMMyselfContact::chatSessionDestroyed( Kopete::ChatSession* session )
{
	m_chatRoomSessions.removeAll( session );
}

void AIMMyselfContact::sendMessage( Kopete::Message& message, Kopete::ChatSession* session )
{
	kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "sending a message" << endl;
	//TODO: remove duplication - factor into a message utils class or something
	Oscar::Message msg;
	QString s;

	if (message.plainBody().isEmpty()) // no text, do nothing
		return;
	//okay, now we need to change the message.escapedBody from real HTML to aimhtml.
	//looking right now for docs on that "format".
	//looks like everything except for alignment codes comes in the format of spans

	//font-style:italic -> <i>
	//font-weight:600 -> <b> (anything > 400 should be <b>, 400 is not bold)
	//text-decoration:underline -> <u>
	//font-family: -> <font face="">
	//font-size:xxpt -> <font ptsize=xx>

	s=message.escapedBody();
	s.replace ( QRegExp( QString::fromLatin1("<span style=\"([^\"]*)\">([^<]*)</span>")),
			QString::fromLatin1("<style>\\1;\"\\2</style>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)font-style:italic;([^\"]*)\"([^<]*)</style>")),
			QString::fromLatin1("<i><style>\\1\\2\"\\3</style></i>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)font-weight:600;([^\"]*)\"([^<]*)</style>")),
			QString::fromLatin1("<b><style>\\1\\2\"\\3</style></b>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)text-decoration:underline;([^\"]*)\"([^<]*)</style>")),
			QString::fromLatin1("<u><style>\\1\\2\"\\3</style></u>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)font-family:([^;]*);([^\"]*)\"([^<]*)</style>")),
			QString::fromLatin1("<font face=\"\\2\"><style>\\1\\3\"\\4</style></font>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)font-size:([^p]*)pt;([^\"]*)\"([^<]*)</style>")),
			QString::fromLatin1("<font ptsize=\"\\2\"><style>\\1\\3\"\\4</style></font>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)color:([^;]*);([^\"]*)\"([^<]*)</style>")),
			QString::fromLatin1("<font color=\"\\2\"><style>\\1\\3\"\\4</style></font>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)\"([^<]*)</style>")),
			QString::fromLatin1("\\2"));

	//okay now change the <font ptsize="xx"> to <font size="xx">

	//0-9 are size 1
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"\\d\">")),
			QString::fromLatin1("<font size=\"1\">"));
	//10-11 are size 2
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"1[01]\">")),
			QString::fromLatin1("<font size=\"2\">"));
	//12-13 are size 3
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"1[23]\">")),
			QString::fromLatin1("<font size=\"3\">"));
	//14-16 are size 4
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"1[456]\">")),
			QString::fromLatin1("<font size=\"4\">"));
	//17-22 are size 5
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"(?:1[789]|2[012])\">")),
			QString::fromLatin1("<font size=\"5\">"));
	//23-29 are size 6
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"2[3456789]\">")),QString::fromLatin1("<font size=\"6\">"));
	//30- (and any I missed) are size 7
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"[^\"]*\">")),QString::fromLatin1("<font size=\"7\">"));

	kDebug(14190) << k_funcinfo << "sending "
		<< s << endl;

	msg.setSender( contactId() );
	msg.setText( Oscar::Message::UserDefined, s, m_acct->defaultCodec() );
	msg.setTimestamp(message.timestamp());
	msg.setType(0x03);
	msg.addProperty( Oscar::Message::ChatRoom );

	AIMChatSession* aimSession = dynamic_cast<AIMChatSession*>( session );
	if ( !aimSession )
	{
		kWarning(OSCAR_AIM_DEBUG) << "couldn't convert to AIM chat room session!" << endl;
		session->messageSucceeded();
		return;
	}
	msg.setExchange( aimSession->exchange() );
	msg.setChatRoom( aimSession->roomName() );

	m_acct->engine()->sendMessage( msg );
	//session->appendMessage( message );
	session->messageSucceeded();
}


AIMAccount::AIMAccount(Kopete::Protocol *parent, QString accountID)
	: OscarAccount(parent, accountID, false)
{
	kDebug(14152) << k_funcinfo << accountID << ": Called."<< endl;
	AIMMyselfContact* mc = new AIMMyselfContact( this );
	setMyself( mc );
	myself()->setOnlineStatus( static_cast<AIMProtocol*>( parent )->statusOffline );
	QString profile = configGroup()->readEntry( "Profile",
			i18n( "Visit the Kopete website at <a href=\"http://kopete.kde.org\">http://kopete.kde.org</a>") );
	mc->setOwnProfile( profile );

	m_joinChatDialog = 0;
	m_visibilityDialog = 0;
	QObject::connect( Kopete::ContactList::self(),
			SIGNAL( globalIdentityChanged( const QString&, const QVariant& ) ),
			this,
			SLOT( slotGlobalIdentityChanged( const QString&, const QVariant& ) ) );

	QObject::connect( engine(), SIGNAL( chatRoomConnected( WORD, const QString& ) ),
			this, SLOT( connectedToChatRoom( WORD, const QString& ) ) );

	QObject::connect( engine(), SIGNAL( userJoinedChat( Oscar::WORD, const QString&, const QString& ) ),
			this, SLOT( userJoinedChat( Oscar::WORD, const QString&, const QString& ) ) );

	QObject::connect( engine(), SIGNAL( userLeftChat( Oscar::WORD, const QString&, const QString& ) ),
			this, SLOT( userLeftChat( Oscar::WORD, const QString&, const QString& ) ) );

	QObject::connect( this, SIGNAL( buddyIconChanged() ), this, SLOT( slotBuddyIconChanged() ) );

}

AIMAccount::~AIMAccount()
{
}

OscarContact *AIMAccount::createNewContact( const QString &contactId, Kopete::MetaContact *parentContact, const OContact& ssiItem )
{
	AIMContact* contact = new AIMContact( this, contactId, parentContact, QString::null, ssiItem );
	if ( !ssiItem.alias().isEmpty() )
		contact->setProperty( Kopete::Global::Properties::self()->nickName(), ssiItem.alias() );

	return contact;
}

QString AIMAccount::sanitizedMessage( const QString& message )
{
	QDomDocument doc;
	QString domError;
	int errLine = 0, errCol = 0;
	doc.setContent( message, false, &domError, &errLine, &errCol );
	if ( !domError.isEmpty() ) //error parsing, do nothing
	{
		kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "error from dom document conversion: "
			<< domError << endl;
		return message;
	}
	else
	{
		kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "conversion to dom document successful."
			<< "looking for font tags" << endl;
		QDomNodeList fontTagList = doc.elementsByTagName( "font" );
		if ( fontTagList.count() == 0 )
		{
			kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "No font tags found. Returning normal message" << endl;
			return message;
		}
		else
		{
			kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "Found font tags. Attempting replacement" << endl;
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
					kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "Found attribute to replace. Doing replacement" << endl;
					QString backgroundColor = fontEl.attribute( "back" );
					backgroundColor.insert( 0, "background-color: " );
					backgroundColor.append( ';' );
					fontEl.setAttribute( "style", backgroundColor );
					fontEl.removeAttribute( "back" );
				}
			}
		}
	}
	kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "sanitized message is " << doc.toString();
	return doc.toString();
}

KActionMenu* AIMAccount::actionMenu()
{
	KActionMenu *mActionMenu = Kopete::Account::actionMenu();

	mActionMenu->addSeparator();

	KAction* actionVisibility = new KToggleAction( i18n( "Set Visibility..." ), 0, "AIMAccount::mActionSetVisibility" );
	QObject::connect( actionVisibility, SIGNAL(triggered(bool)), this, SLOT(slotSetVisiblility()) );
	mActionMenu->addAction( actionVisibility );

	KAction* m_joinChatAction = new KAction( i18n( "Join Chat..." ), 0, "join_a_chat" );
	QObject::connect( m_joinChatAction, SIGNAL(triggered(bool)), this, SLOT(slotJoinChat()) );
	mActionMenu->addAction( m_joinChatAction );

	KAction* m_editInfoAction = new KAction( KIcon("identity"), i18n( "Edit User Info..." ), 0, "actionEditInfo" );
	QObject::connect( m_editInfoAction, SIGNAL(triggered(bool)), this, SLOT(slotEditInfo()) );
	mActionMenu->addAction( m_editInfoAction );

	return mActionMenu;
}

void AIMAccount::setAway(bool away, const QString &awayReason)
{
	//kDebug(14152) << k_funcinfo << accountId() << "reason is " << awayReason << endl;
	if ( away )
	{
		engine()->setStatus( Client::Away, awayReason );
		AIMMyselfContact* me = static_cast<AIMMyselfContact *> ( myself() );
		me->setLastAwayMessage(awayReason);
		me->setProperty( Kopete::Global::Properties::self()->statusMessage(), awayReason );
	}
	else
	{
		engine()->setStatus( Client::Online );
		AIMMyselfContact* me = static_cast<AIMMyselfContact *> ( myself() );
		me->setLastAwayMessage(QString::null);
		me->removeProperty( Kopete::Global::Properties::self()->statusMessage() );
	}
}

void AIMAccount::setOnlineStatus( const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason )
{
	kDebug(14152) << k_funcinfo << "called with reason = '" << reason.message() <<"' status = " << status.status() << endl;
	if ( status.status() == Kopete::OnlineStatus::Offline )
		disconnect();
	else if ( myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline )
	{ //account isn't online, so we'll need to connect. 
		//FIXME: what if we're in the middle of connecting?
		kDebug(14152) << k_funcinfo << accountId() << " was offline. time to connect" << endl;
		//ah, but is there a to set away as well? TODO
		OscarAccount::connect();
	}
	else
	{ //we're just changing our away-ness and possibly a message.
		if ( status.status() == Kopete::OnlineStatus::Away )
			setAway( true, reason.message() );
		else
			setAway( false );
	}
}

void AIMAccount::setStatusMessage( const Kopete::StatusMessage& statusMessage )
{
}

void AIMAccount::setUserProfile(const QString &profile)
{
	kDebug(14152) << k_funcinfo << "called." << endl;
	AIMMyselfContact* aimmc = dynamic_cast<AIMMyselfContact*>( myself() );
	if ( aimmc )
		aimmc->setOwnProfile( profile );
	configGroup()->writeEntry( QString::fromLatin1( "Profile" ), profile );
}

void AIMAccount::slotEditInfo()
{
	if ( !isConnected() )
	{
		KMessageBox::sorry( Kopete::UI::Global::mainWidget(),
				i18n( "Editing your user info is not possible because "
					"you are not connected." ),
				i18n( "Unable to edit user info" ) );
		return;
	}
	AIMUserInfoDialog *myInfo = new AIMUserInfoDialog(static_cast<AIMContact *>( myself() ), this);
	myInfo->exec(); // This is a modal dialog
}

void AIMAccount::slotGlobalIdentityChanged( const QString& key, const QVariant& value )
{
	//do something with the photo
	kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "Global identity changed" << endl;
	kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "key: " << key << endl;
	kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "value: " << value << endl;

	if( !configGroup()->readEntry("ExcludeGlobalIdentity", false) )
	{
		if ( key == Kopete::Global::Properties::self()->nickName().key() )
		{
			//edit ssi item to change alias (if possible)
		}

		if ( key == Kopete::Global::Properties::self()->photo().key() )
		{
			setBuddyIcon( value.toString() );
		}
	}
}

void AIMAccount::slotBuddyIconChanged()
{
	// need to disconnect because we could end up with many connections
	QObject::disconnect( engine(), SIGNAL( iconServerConnected() ), this, SLOT( slotBuddyIconChanged() ) );
	if ( !engine()->isActive() )
	{
		QObject::connect( engine(), SIGNAL( iconServerConnected() ), this, SLOT( slotBuddyIconChanged() ) );
		return;
	}

	QString photoPath = myself()->property( Kopete::Global::Properties::self()->photo() ).value().toString();

	ContactManager* ssi = engine()->ssiManager();
	OContact item = ssi->findItemForIconByRef( 1 );

	if ( photoPath.isEmpty() )
	{
		if ( item )
		{
			kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "Removing icon hash item from ssi" << endl;
			OContact s(item);

			//remove hash and alias
			QList<TLV> tList( item.tlvList() );
			TLV t = Oscar::findTLV( tList, 0x00D5 );
			if ( t )
				tList.removeAll( t );

			item.setTLVList( tList );
			//s is old, item is new. modification will occur
			engine()->modifyContactItem( s, item );
		}
	}
	else
	{
		QFile iconFile( photoPath );
		iconFile.open( QIODevice::ReadOnly );

		KMD5 iconHash;
		iconHash.update( iconFile );
		kDebug(OSCAR_AIM_DEBUG) << k_funcinfo  << "hash is :" << iconHash.hexDigest() << endl;

		//find old item, create updated item
		if ( !item )
		{
			kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "no existing icon hash item in ssi. creating new" << endl;

			TLV t;
			t.type = 0x00D5;
			t.data.resize( 18 );
			t.data[0] = 0x00;
			t.data[1] = 0x10;
			memcpy(t.data.data() + 2, iconHash.rawDigest(), 16);
			t.length = t.data.size();

			QList<Oscar::TLV> list;
			list.append( t );

			OContact s( "1", 0, ssi->nextContactId(), ROSTER_BUDDYICONS, list );

			//item is a non-valid ssi item, so the function will add an item
			kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "setting new icon item" << endl;
			engine()->modifyContactItem( item, s );
		}
		else
		{ //found an item
			OContact s(item);
			kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "modifying old item in ssi." << endl;
			QList<TLV> tList( item.tlvList() );

			TLV t = Oscar::findTLV( tList, 0x00D5 );
			if ( t )
				tList.removeAll( t );
			else
				t.type = 0x00D5;

			t.data.resize( 18 );
			t.data[0] = 0x00;
			t.data[1] = 0x10;
			memcpy(t.data.data() + 2, iconHash.rawDigest(), 16);
			t.length = t.data.size();
			tList.append( t );

			item.setTLVList( tList );
			//s is old, item is new. modification will occur
			engine()->modifyContactItem( s, item );
		}
		iconFile.close();
	}
}

void AIMAccount::slotJoinChat()
{
	if ( !isConnected() )
	{
		KMessageBox::sorry( Kopete::UI::Global::mainWidget(),
				i18n( "Joining an AIM chat room is not possible because "
					"you are not connected." ),
				i18n( "Unable to Join AIM Chat Room" ) );
		return;
	}

	//get the exchange info
	//create the dialog
	//join the chat room
	if ( !m_joinChatDialog )
	{
		m_joinChatDialog = new AIMJoinChatUI( this, Kopete::UI::Global::mainWidget() );
		QObject::connect( m_joinChatDialog, SIGNAL( closing( int ) ),
				this, SLOT( joinChatDialogClosed( int ) ) );
		QList<int> list = engine()->chatExchangeList();
		m_joinChatDialog->setExchangeList( list );
		m_joinChatDialog->show();
	}
	else
		m_joinChatDialog->raise();
}

void AIMAccount::joinChatDialogClosed( int code )
{
	if ( code == QDialog::Accepted )
	{
		//join the chat
		kDebug(14152) << k_funcinfo << "chat accepted." << endl;
		engine()->joinChatRoom( m_joinChatDialog->roomName(),
				m_joinChatDialog->exchange().toInt() );
	}

	m_joinChatDialog->delayedDestruct();
	m_joinChatDialog = 0L;
}

void AIMAccount::loginActions()
{
	OscarAccount::loginActions();

	using namespace AIM::PrivacySettings;
	int privacySetting = this->configGroup()->readEntry( "PrivacySetting", int(AllowAll) );
	this->setPrivacySettings( privacySetting );
}

void AIMAccount::disconnected( DisconnectReason reason )
{
	kDebug( OSCAR_AIM_DEBUG ) << k_funcinfo << "Attempting to set status offline" << endl;
	myself()->setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusOffline );

	QHash<QString, Kopete::Contact*> contactList = contacts();
	foreach( Kopete::Contact* c, contactList.values() )
	{
		OscarContact* oc = static_cast<OscarContact*>( c );
		if ( oc )
			oc->setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusOffline );
	}
	
	OscarAccount::disconnected( reason );
}

void AIMAccount::messageReceived( const Oscar::Message& message )
{
	kDebug(14152) << k_funcinfo << " Got a message, calling OscarAccount::messageReceived" << endl;
	// Want to call the parent to do everything else
	if ( message.type() != 0x0003 )
	{
		OscarAccount::messageReceived(message);

		// Check to see if our status is away, and send an away message
		// Might be duplicate code from the parent class to get some needed information
		// Perhaps a refactoring is needed.
		kDebug(14152) << k_funcinfo << "Checking to see if I'm online.." << endl;
		if( myself()->onlineStatus().status() == Kopete::OnlineStatus::Away )
		{
			QString sender = Oscar::normalize( message.sender() );
			AIMContact* aimSender = static_cast<AIMContact *> ( contacts()[sender] ); //should exist now
			if ( !aimSender )
			{
				kWarning(OSCAR_RAW_DEBUG) << "For some reason, could not get the contact "
					<< "That this message is from: " << message.sender() << ", Discarding message" << endl;
				return;
			}
			// Create, or get, a chat session with the contact
			Kopete::ChatSession* chatSession = aimSender->manager( Kopete::Contact::CanCreate );

			// get the away message we have set
			AIMMyselfContact* myContact = static_cast<AIMMyselfContact *> ( myself() );
			QString msg = myContact->lastAwayMessage();
			kDebug(14152) << k_funcinfo << "Got away message: " << msg << endl;
			// Create the message
			Kopete::Message chatMessage( myself(), aimSender, msg, Kopete::Message::Outbound,
					Kopete::Message::RichText );
			kDebug(14152) << k_funcinfo << "Sending autoresponse" << endl;
			// Send the message
			aimSender->sendAutoResponse( chatMessage );
		}
	}
	else
	{
		kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "have chat message" << endl;
		//handle chat room messages seperately
		QList<Kopete::ChatSession*> chats = Kopete::ChatSessionManager::self()->sessions();
		QList<Kopete::ChatSession*>::iterator it,  itEnd = chats.end();
		for ( it = chats.begin(); it != itEnd; ++it )
		{
			Kopete::ChatSession* kcs = ( *it );
			AIMChatSession* session = dynamic_cast<AIMChatSession*>( kcs );
			if ( !session )
				continue;

			if ( session->exchange() == message.exchange() &&
					Oscar::normalize( session->roomName() ) ==
					Oscar::normalize( message.chatRoom() ) )
			{
				kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "found chat session for chat room" << endl;
				Kopete::Contact* ocSender = contacts()[Oscar::normalize( message.sender() )];
				//sanitize;
				QString sanitizedMsg = sanitizedMessage( message.text( defaultCodec() ) );

				Kopete::ContactPtrList me;
				me.append( myself() );
				Kopete::Message chatMessage( message.timestamp(), ocSender, me, sanitizedMsg,
						Kopete::Message::Inbound, Kopete::Message::RichText );

				session->appendMessage( chatMessage );
			}
		}
	}
}

void AIMAccount::connectedToChatRoom( WORD exchange, const QString& room )
{
	kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "Creating chat room session" << endl;
	Kopete::ContactPtrList emptyList;
	AIMMyselfContact* me = static_cast<AIMMyselfContact*>( myself() );
	AIMChatSession* session = static_cast<AIMChatSession*>( me->manager( Kopete::Contact::CanCreate,
				exchange, room ) );
	session->setDisplayName( room );
	if ( session->view( true ) )
		session->raiseView();
}

void AIMAccount::userJoinedChat( WORD exchange, const QString& room, const QString& contact )
{
	if ( Oscar::normalize( contact ) == Oscar::normalize( myself()->contactId() ) )
		return;

	kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "user " << contact << " has joined the chat" << endl;
	QList<Kopete::ChatSession*> chats = Kopete::ChatSessionManager::self()->sessions();
	QList<Kopete::ChatSession*>::iterator it, itEnd = chats.end();
	for ( it = chats.begin(); it != itEnd; ++it )
	{
		Kopete::ChatSession* kcs = ( *it );
		AIMChatSession* session = dynamic_cast<AIMChatSession*>( kcs );
		if ( !session )
			continue;

		kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << session->exchange() << " " << exchange << endl;
		kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << session->roomName() << " " << room << endl;
		if ( session->exchange() == exchange && session->roomName() == room )
		{
			kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "found correct chat session" << endl;
			Kopete::Contact* c;
			if ( contacts()[Oscar::normalize( contact )] )
				c = contacts()[Oscar::normalize( contact )];
			else
			{
				Kopete::MetaContact* mc = addContact( Oscar::normalize( contact ),
						contact, 0, Kopete::Account::Temporary );
				if ( !mc )
					kWarning(OSCAR_AIM_DEBUG) << "Unable to add contact for chat room" << endl;

				c = mc->contacts().first();
				c->setNickName( contact );
			}

			kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "adding contact" << endl;
			session->addContact( c, static_cast<AIMProtocol*>( protocol() )->statusOnline, true /* suppress */ );
		}
	}
}

void AIMAccount::userLeftChat( WORD exchange, const QString& room, const QString& contact )
{
	if ( Oscar::normalize( contact ) == Oscar::normalize( myself()->contactId() ) )
		return;

	QList<Kopete::ChatSession*> chats = Kopete::ChatSessionManager::self()->sessions();
	QList<Kopete::ChatSession*>::iterator it, itEnd = chats.end();
	for ( it = chats.begin(); it != itEnd; ++it )
	{
		Kopete::ChatSession* kcs = ( *it );
		AIMChatSession* session = dynamic_cast<AIMChatSession*>( kcs );
		if ( !session )
			continue;

		if ( session->exchange() == exchange && session->roomName() == room )
		{
			//delete temp contact
			Kopete::Contact* c = contacts()[Oscar::normalize( contact )];
			if ( !c )
			{
				kWarning(OSCAR_AIM_DEBUG) << k_funcinfo << "couldn't find the contact that's left the chat!" << endl;
				continue;
			}
			session->removeContact( c );
			Kopete::MetaContact* mc = c->metaContact();
			if ( mc->isTemporary() )
			{
				mc->removeContact( c );
				delete c;
				delete mc;
			}
		}
	}
}


void AIMAccount::connectWithPassword( const QString & )
{
	kDebug(14152) << k_funcinfo << "accountId='" << accountId() << "'" << endl;

	// Get the screen name for this account
	QString screenName = accountId();
	QString server = configGroup()->readEntry( "Server", QString::fromLatin1( "login.oscar.aol.com" ) );
	uint port = configGroup()->readEntry( "Port", 5190 );

	Connection* c = setupConnection( server, port );

	QString _password = password().cachedValue();
	if ( _password.isEmpty() )
	{
		kDebug(14150) << "Kopete is unable to attempt to sign-on to the "
			<< "AIM network because no password was specified in the "
			<< "preferences." << endl;
	}
	else if ( myself()->onlineStatus() == static_cast<AIMProtocol*>( protocol() )->statusOffline )
	{
		kDebug(14152) << k_funcinfo << "Logging in as " << accountId() << endl ;
		engine()->start( server, port, accountId(), _password.left(16) );
		engine()->connectToServer( c, server, true /* doAuth */ );
		myself()->setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusConnecting );
	}
}

void AIMAccount::slotSetVisiblility()
{
	if( !isConnected() )
	{
		KMessageBox::sorry( Kopete::UI::Global::mainWidget(),
				i18n("You must be online to set users visibility."),
				i18n("ICQ Plugin") );
		return;
	}

	if ( !m_visibilityDialog )
	{
		m_visibilityDialog = new OscarVisibilityDialog( engine(), Kopete::UI::Global::mainWidget() );
		QObject::connect( m_visibilityDialog, SIGNAL( closing() ),
				this, SLOT( slotVisibilityDialogClosed() ) );

		//add all contacts;
		OscarVisibilityDialog::ContactMap contactMap;
		QMap<QString, QString> revContactMap;

		QList<OContact> contactList = engine()->ssiManager()->contactList();
		QList<OContact>::const_iterator it, cEnd = contactList.constEnd();

		for ( it = contactList.constBegin(); it != cEnd; ++it )
		{
			QString contactId = ( *it ).name();

			OscarContact* oc = dynamic_cast<OscarContact*>( contacts()[( *it ).name()] );
			if ( oc )
			{
				contactMap.insert( oc->nickName(), contactId );
				revContactMap.insert( contactId, oc->nickName() );
			}
			else
			{
				contactMap.insert( contactId, contactId );
				revContactMap.insert( contactId, contactId );
			}
		}
		m_visibilityDialog->addContacts( contactMap );

		//add contacts from visible list
		QStringList tmpList;

		contactList = engine()->ssiManager()->visibleList();
		cEnd = contactList.constEnd();

		for ( it = contactList.constBegin(); it != cEnd; ++it )
			tmpList.append( revContactMap[( *it ).name()] );

		m_visibilityDialog->addVisibleContacts( tmpList );

		//add contacts from invisible list
		tmpList.clear();

		contactList = engine()->ssiManager()->invisibleList();
		cEnd = contactList.constEnd();

		for ( it = contactList.constBegin(); it != cEnd; ++it )
			tmpList.append( revContactMap[( *it ).name()] );

		m_visibilityDialog->addInvisibleContacts( tmpList );

		m_visibilityDialog->resize( 550, 350 );
		m_visibilityDialog->show();
	}
	else
	{
		m_visibilityDialog->raise();
	}
}

void AIMAccount::slotVisibilityDialogClosed()
{
	m_visibilityDialog->deleteLater();
	m_visibilityDialog = 0L;
}

void AIMAccount::setPrivacySettings( int privacy )
{
	using namespace AIM::PrivacySettings;

	BYTE privacyByte = 0x01;
	DWORD userClasses = 0xFFFFFFFF;

	switch ( privacy )
	{
		case AllowAll:
			privacyByte = 0x01;
			break;
		case BlockAll:
			privacyByte = 0x02;
			break;
		case AllowPremitList:
			privacyByte = 0x03;
			break;
		case BlockDenyList:
			privacyByte = 0x04;
			break;
		case AllowMyContacts:
			privacyByte = 0x05;
			break;
		case BlockAIM:
			privacyByte = 0x01;
			userClasses = 0x00000004;
			break;
	}

	this->setPrivacyTLVs( privacyByte, userClasses );
}

void AIMAccount::setPrivacyTLVs( BYTE privacy, DWORD userClasses )
{
	ContactManager* ssi = engine()->ssiManager();
	OContact item = ssi->findItem( QString::null, ROSTER_VISIBILITY );

	QList<Oscar::TLV> tList;

	tList.append( TLV( 0x00CA, 1, (char *)&privacy ) );
	tList.append( TLV( 0x00CB, sizeof(userClasses), (char *)&userClasses ) );

	if ( !item )
	{
		kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "Adding new privacy TLV item" << endl;
		OContact s( QString::null, 0, ssi->nextContactId(), ROSTER_VISIBILITY, tList );
		engine()->modifyContactItem( item, s );
	}
	else
	{ //found an item
		OContact s(item);

		if ( Oscar::updateTLVs( s, tList ) == true )
		{
			kDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "Updating privacy TLV item" << endl;
			engine()->modifyContactItem( item, s );
		}
	}
}

#include "aimaccount.moc"
//kate: tab-width 4; indent-mode csands;
