/*
    yahooaccount.cpp - Manages a single Yahoo account

    Copyright (c) 2003 by Gav Wood               <gav@kde.org>
    Copyright (c) 2003-2004 by Matt Rogers       <matt.rogers@kdemail.net>
    Based on code by Olivier Goffart             <ogoffart@tiscalinet.be>
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
//Standard Header
#include <ctime>

//QT
#include <qfont.h>
#include <qdatetime.h>
#include <qcolor.h>
#include <qregexp.h>

// KDE
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <kapplication.h>

// Kopete
#include <kopetemessagemanager.h>
#include <kopetemessage.h>
#include <kopetepassword.h>
#include <kopeteuiglobal.h>
#include <kopetenotifyclient.h>
#include <kopetemetacontact.h>
#include <kopetecontactlist.h>

// Yahoo
#include "yahooaccount.h"
#include "yahoocontact.h"

YahooAwayDialog::YahooAwayDialog(YahooAccount* account, QWidget *parent, const char *name) :
	KopeteAwayDialog(parent, name)
{
	theAccount = account;
}

void YahooAwayDialog::setAway(int awayType)
{
	theAccount->setAway(awayType, getSelectedAwayMessage());
}


YahooAccount::YahooAccount(YahooProtocol *parent, const QString& accountId, const char *name)
 : Kopete::PasswordedAccount(parent, accountId, 0, name)
{

	// first things first - initialise internals
	theHaveContactList = false;
	stateOnConnection = 0;
	theAwayDialog = new YahooAwayDialog( this );
	m_protocol = parent;
	m_session = 0;
	m_lastDisconnectCode = 0;
	m_currentMailCount = 0;

	setMyself( new YahooContact( this, accountId, accountId, 0 ) );
	static_cast<YahooContact *>( myself() )->setOnlineStatus( parent->Offline );

	QObject::connect( this, SIGNAL( needReconnect() ), this, SLOT( slotNeedReconnect() ) );

}

YahooAccount::~YahooAccount()
{
	delete theAwayDialog;
}

void YahooAccount::slotGoStatus( int status, const QString &awayMessage)
{
	kdDebug(14180) << k_funcinfo << endl;

	if( !isConnected() )
	{
		connect( m_protocol->statusFromYahoo( status ) );
		stateOnConnection = status;
	}
	else
	{
		m_session->setAway( yahoo_status( status ), awayMessage, status ? 1 : 0 );
		myself()->setOnlineStatus( m_protocol->statusFromYahoo( status ) );
	}
}

void YahooAccount::loaded()
{
	kdDebug(14180) << k_funcinfo << endl;
	QString newPluginData;

	newPluginData = pluginData(m_protocol, QString::fromLatin1("displayName"));
	if(!newPluginData.isEmpty())
		myself()->rename(newPluginData);	//TODO: might cause problems depending on rename semantics
}

YahooSession *YahooAccount::yahooSession()
{
	return m_session ? m_session : 0L;
}

QString YahooAccount::stripMsgColorCodes(const QString& msg)
{
	QString filteredMsg = msg;
	filteredMsg.remove(QRegExp("\033\\[(..m|#......)"));

	//Handle bold, underline and italic messages
	filteredMsg.replace( QRegExp("\033\\[1m"), "<b>" );
	filteredMsg.replace( QRegExp("\033\\[x1m"), "</b>" );
	//work around gaim's broken sending
	filteredMsg.remove( QRegExp( "\033\\[xlm" ) );
	filteredMsg.remove( QRegExp( "\033\\[lm" ) );
	//end work around
	filteredMsg.replace( QRegExp("\033\\[3m"), "<i>" );
	filteredMsg.replace( QRegExp("\033\\[x3m"), "</i>" );
	filteredMsg.replace( QRegExp("\033\\[4m"), "<u>" );
	filteredMsg.replace( QRegExp("\033\\[x4m"), "</u>" );
	return filteredMsg;
}

QColor YahooAccount::getMsgColor(const QString& msg)
{
	/* Yahoo sends a message either with color or without color
	 * so we have to use this really hacky method to get colors
	 */
	//kdDebug(14180) << k_funcinfo << "msg is " << msg << endl;
	//Please note that some of the colors are hard-coded to
	//match the yahoo colors
	if ( msg.find("\033[38m") != -1 )
		return Qt::red;
	if ( msg.find("\033[34m") != -1 )
		return Qt::green;
	if ( msg.find("\033[31m") != -1 )
		return Qt::blue;
	if ( msg.find("\033[39m") != -1 )
		return Qt::yellow;
	if ( msg.find("\033[36m") != -1 )
		return Qt::darkMagenta;
	if ( msg.find("\033[32m") != -1 )
		return Qt::cyan;
	if ( msg.find("\033[37m") != -1 )
		return QColor("#FFAA39");
	if ( msg.find("\033[35m") != -1 )
		return QColor("#FFD8D8");
	if ( msg.find("\033[#") != -1 )
	{
		kdDebug(14180) << "Custom color is " << msg.mid(msg.find("\033[#")+2,7) << endl;
		return QColor(msg.mid(msg.find("\033[#")+2,7));
	}

	//return a default value just in case
	return Qt::black;
}

void YahooAccount::initConnectionSignals( enum SignalConnectionType sct )
{
	if ( !m_session )
		return;

	if ( sct == MakeConnections )
	{
		QObject::connect(m_session, SIGNAL(loginResponse(int, const QString &)),
				this, SLOT(slotLoginResponse(int, const QString &)) );

		QObject::connect(m_session, SIGNAL(gotBuddy(const QString &, const QString &, const QString &)),
				this, SLOT(slotGotBuddy(const QString &, const QString &, const QString &)));

		QObject::connect(m_session, SIGNAL( buddyListFetched( int ) ),
				this, SLOT(slotBuddyListFetched( int ) ) );

		QObject::connect(m_session, SIGNAL(statusChanged(const QString&, int, const QString&, int)),
				this, SLOT(slotStatusChanged(const QString&, int, const QString&, int)));

		QObject::connect(m_session, SIGNAL(gotIm(const QString&, const QString&, long, int)),
				this, SLOT(slotGotIm(const QString &, const QString&, long, int)));

		QObject::connect(m_session, SIGNAL(gotConfInvite( const QString&, const QString&, const QString&,
				const QStringList&)), this,
				SLOT(slotGotConfInvite(const QString&, const QString&, const QString&, const QStringList&)));

		QObject::connect(m_session, SIGNAL(confUserDecline(const QString&, const QString &, const QString &)), this,
				SLOT(slotConfUserDecline( const QString &, const QString &, const QString &)) );

		QObject::connect(m_session , SIGNAL(confUserJoin( const QString &, const QString &)), this,
				SLOT(slotConfUserJoin( const QString &, const QString &)) );

		QObject::connect(m_session , SIGNAL(confUserLeave( const QString &, const QString &)), this,
				SLOT(slotConfUserLeave( const QString &, const QString &)) );

		QObject::connect(m_session , SIGNAL(confMessage( const QString &, const QString &, const QString &)), this,
				SLOT(slotConfMessage( const QString &, const QString &, const QString &)) );

		QObject::connect(m_session,
				SIGNAL(gotFile(const QString &, const QString &, long, const QString &, const QString &, unsigned long)),
				this,
				SLOT(slotGotFile(const QString&, const QString&, long, const QString&, const QString&, unsigned long)));

		QObject::connect(m_session , SIGNAL(contactAdded(const QString &, const QString &, const QString &)), this,
				SLOT(slotContactAdded(const QString &, const QString &, const QString &)));

		QObject::connect(m_session , SIGNAL(rejected(const QString &, const QString &)), this,
				SLOT(slotRejected(const QString&, const QString&)));

		QObject::connect(m_session, SIGNAL(typingNotify(const QString &, int)), this ,
				SLOT(slotTypingNotify(const QString &, int)));

		QObject::connect(m_session, SIGNAL(gameNotify(const QString &, int)), this,
				SLOT(slotGameNotify( const QString &, int)));

		QObject::connect(m_session, SIGNAL(mailNotify(const QString&, const QString&, int)), this,
				SLOT(slotMailNotify(const QString &, const QString&, int)));

		QObject::connect(m_session, SIGNAL(systemMessage(const QString&)), this,
				SLOT(slotSystemMessage(const QString &)));

		QObject::connect(m_session, SIGNAL(error(const QString&, int)), this,
				SLOT(slotError(const QString &, int )));

		QObject::connect(m_session, SIGNAL(gotIdentities(const QStringList &)), this,
				SLOT(slotGotIdentities( const QStringList&)));
	}

	if ( sct == DeleteConnections )
	{
		QObject::disconnect(m_session, SIGNAL(loginResponse(int, const QString &)),
				this, SLOT(slotLoginResponse(int, const QString &)) );

		QObject::disconnect(m_session, SIGNAL(gotBuddy(const QString &, const QString &, const QString &)),
				this, SLOT(slotGotBuddy(const QString &, const QString &, const QString &)));

		QObject::disconnect(m_session, SIGNAL( buddyListFetched( int ) ),
					this, SLOT(slotBuddyListFetched( int ) ) );

		QObject::disconnect(m_session, SIGNAL(statusChanged(const QString&, int, const QString&, int)),
				this, SLOT(slotStatusChanged(const QString&, int, const QString&, int)));

		QObject::disconnect(m_session, SIGNAL(gotIm(const QString&, const QString&, long, int)),
				this, SLOT(slotGotIm(const QString &, const QString&, long, int)));

		QObject::disconnect(m_session, SIGNAL(gotConfInvite( const QString&, const QString&, const QString&,
				const QStringList&)), this,
				SLOT(slotGotConfInvite(const QString&, const QString&, const QString&, const QStringList&)));

		QObject::disconnect(m_session, SIGNAL(confUserDecline(const QString&, const QString &, const QString &)), this,
				SLOT(slotConfUserDecline( const QString &, const QString &, const QString &)) );

		QObject::disconnect(m_session , SIGNAL(confUserJoin( const QString &, const QString &)), this,
				SLOT(slotConfUserJoin( const QString &, const QString &)) );

		QObject::disconnect(m_session , SIGNAL(confUserLeave( const QString &, const QString &)), this,
				SLOT(slotConfUserLeave( const QString &, const QString &)) );

		QObject::disconnect(m_session , SIGNAL(confMessage( const QString &, const QString &, const QString &)), this,
				SLOT(slotConfMessage( const QString &, const QString &, const QString &)) );

		QObject::disconnect(m_session,
				SIGNAL(gotFile(const QString &, const QString &, long, const QString &, const QString &, unsigned long)),
				this,
				SLOT(slotGotFile(const QString&, const QString&, long, const QString&, const QString&, unsigned long)));

		QObject::disconnect(m_session , SIGNAL(contactAdded(const QString &, const QString &, const QString &)), this,
				SLOT(slotContactAdded(const QString &, const QString &, const QString &)));

		QObject::disconnect(m_session , SIGNAL(rejected(const QString &, const QString &)), this,
				SLOT(slotRejected(const QString&, const QString&)));

		QObject::disconnect(m_session, SIGNAL(typingNotify(const QString &, int)), this ,
				SLOT(slotTypingNotify(const QString &, int)));

		QObject::disconnect(m_session, SIGNAL(gameNotify(const QString &, int)), this,
				SLOT(slotGameNotify( const QString &, int)));

		QObject::disconnect(m_session, SIGNAL(mailNotify(const QString&, const QString&, int)), this,
				SLOT(slotMailNotify(const QString &, const QString&, int)));

		QObject::disconnect(m_session, SIGNAL(systemMessage(const QString&)), this,
				SLOT(slotSystemMessage(const QString &)));

		QObject::disconnect(m_session, SIGNAL(error(const QString&, int)), this,
				SLOT(slotError(const QString &, int )));

		QObject::disconnect(m_session, SIGNAL(gotIdentities(const QStringList &)), this,
				SLOT(slotGotIdentities( const QStringList&)));
	}
}

void YahooAccount::connectWithPassword( const QString &passwd )
{
	if ( passwd.isNull() )
	{ //cancel the connection attempt
		static_cast<YahooContact*>( myself() )->setOnlineStatus( m_protocol->Offline );
		return;
	}

	QString server = "scs.msg.yahoo.com";
	int port = 5050;

	/* call loaded() here. It shouldn't hurt anything
	 * and ensures that all our settings get loaded */
	loaded();

	YahooSessionManager::manager()->setPager( server, port );

	m_session = YahooSessionManager::manager()->createSession( accountId(), passwd );
	if(!isConnected())
	{
		kdDebug(14180) << "Attempting to connect to Yahoo on <" << server << ":" << port << ">. user <" << accountId() << ">" << endl;

		static_cast<YahooContact*>( myself() )->setOnlineStatus( m_protocol->Connecting );
		if ( m_session && m_session->sessionId() > 0 )
		{

			initConnectionSignals( MakeConnections );
			kdDebug(14180) << "Starting the login connection" << endl;
			m_session->login( initialStatus().internalStatus() );

		}
		else
		{
			kdDebug(14180) << "Couldn't connect!" << endl;
			// TODO: message box saying can't connect?
		}
	}
	else if ( isAway() )
	{	// They're really away, and they want to un-away.
		slotGoOnline();
	}
	else	// ignore
		kdDebug(14180) << "Yahoo plugin: Ignoring connect request (already connected)." <<endl;
}

void YahooAccount::disconnect()
{
//	kdDebug(14180) << k_funcinfo << endl;

	m_currentMailCount = 0;
	if ( isConnected() )
	{
		kdDebug(14180) <<  "Attempting to disconnect from Yahoo server " << endl;

		m_session->logOff();
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );

		for ( QDictIterator<Kopete::Contact> i( contacts() ); i.current(); ++i )
			static_cast<YahooContact *>( i.current() )->setOnlineStatus( m_protocol->Offline );
	}
	else
	{       //make sure we set everybody else offline explicitly, just for cleanup
		kdDebug(14180) << "Ignoring disconnect request (not fully connected)." << endl;

		for ( QDictIterator<Kopete::Contact> i(contacts()); i.current(); ++i )
			static_cast<YahooContact*>( i.current() )->setOnlineStatus( m_protocol->Offline );
	}

	initConnectionSignals( DeleteConnections );
	theHaveContactList = false;
}

void YahooAccount::setAway(bool status, const QString &awayMessage)
{
	kdDebug(14180) << k_funcinfo << endl;

	if( awayMessage.isEmpty() )
		slotGoStatus( status ? 2 : 0 );
	else
		slotGoStatus( status ? 99 : 0, awayMessage );
}

void YahooAccount::slotConnected()
{
	kdDebug(14180) << k_funcinfo << "Moved to slotLoginResponse for the moment" << endl;
}

void YahooAccount::slotGoOnline()
{
	if( !isConnected() )
		connect( m_protocol->Online );
	else
		slotGoStatus(0);
}

void YahooAccount::slotGoOffline()
{
	if ( isConnected() )
		disconnect();
	else
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
}

void YahooAccount::slotBuddyListFetched( int numBuddies )
{
	kdDebug(14180) << "Number of buddies: " << numBuddies << endl;
	theHaveContactList = true;
}

KActionMenu *YahooAccount::actionMenu()
{
//	kdDebug(14180) << k_funcinfo << endl;
	//TODO: Use a QSignalMapper so all the slots can be consolidated into one function

	KActionMenu *theActionMenu = Kopete::Account::actionMenu();

	return theActionMenu;
}

void YahooAccount::slotGotBuddies( const YList */*theList*/ )
{
	kdDebug(14180) << k_funcinfo << endl;
}

YahooContact *YahooAccount::contact( const QString &id )
{
	return static_cast<YahooContact *>(contacts()[id]);
}

bool YahooAccount::createContact(const QString &contactId, Kopete::MetaContact *parentContact )
{
	kdDebug(14180) << k_funcinfo << " contactId: " << contactId << endl;

	if(!contact(contactId))
	{
		// FIXME: New Contacts are NOT added to KABC, because:
		// How on earth do you tell if a contact is being deserialised or added brand new here?
			// -- actualy (oct 2004) this method is only called when new contact are added.  but this will
			//    maybe change and you will be noticed   --Olivier
		YahooContact *newContact = new YahooContact( this, contactId, parentContact->displayName(), parentContact );
		return newContact != 0;
	}
	else
		kdDebug(14180) << k_funcinfo << "Contact already exists" << endl;

	return false;
}


void YahooAccount::slotNeedReconnect()
{
	password().setWrong();
	connect();
}


/***************************************************************************
 *                                                                         *
 *   Slot for KYahoo signals                                               *
 *                                                                         *
 ***************************************************************************/

void YahooAccount::slotLoginResponse( int succ , const QString &url )
{
	kdDebug(14180) << k_funcinfo << succ << ", " << url << ")]" << endl;
	QString errorMsg;
	if ( succ == YAHOO_LOGIN_OK || (succ == YAHOO_LOGIN_DUPL && m_lastDisconnectCode == 2) )
	{
		slotGotBuddies(yahooSession()->getLegacyBuddyList());

		//Yahoo only supports connecting as invisible and online, nothing else
		if ( initialStatus() == m_protocol->Invisible )
			static_cast<YahooContact *>( myself() )->setOnlineStatus( initialStatus() );
		else
			static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Online );

		m_lastDisconnectCode = 0;
		return;
	}
	else if(succ == YAHOO_LOGIN_PASSWD)
	{
		disconnect();
		emit needReconnect();
		return;
	}
	else if(succ == YAHOO_LOGIN_LOCK)
	{
		errorMsg = i18n("Could not log into Yahoo service: your account has been locked.\nVisit %1 to reactivate it.").arg(url);
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(), KMessageBox::Error, errorMsg);
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
		return;
	}
	else if ( succ == YAHOO_LOGIN_UNAME )
	{
		errorMsg = i18n("Could not log into the Yahoo service: the username specified was invalid.");
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(), KMessageBox::Error, errorMsg);
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
	}
	else if ( succ == YAHOO_LOGIN_DUPL && m_lastDisconnectCode != 2 )
	{
		errorMsg = i18n("You have been logged out of the Yahoo service, possibly due to a duplicate login.");
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(), KMessageBox::Error, errorMsg);
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
		return;
	}

	//If we get here, something went wrong, so set ourselves to offline
	static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );

}

void YahooAccount::slotGotBuddy( const QString &userid, const QString &alias, const QString &group )
{
	kdDebug(14180) << k_funcinfo << endl;
	IDs[userid] = QPair<QString, QString>(group, alias);

	// Serverside -> local
	if ( !contact( userid ) )
	{
		kdDebug(14180) << "SS Contact " << userid << " is not in the contact list. Adding..." << endl;
		Kopete::Group *g=Kopete::ContactList::self()->findGroup(group);
		addMetaContact(userid, alias.isEmpty() ? userid : alias, g, Kopete::Account::ChangeKABC);
	}

}

void YahooAccount::slotGotIgnore( const QStringList & /* igns */ )
{
	//kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotGotIdentities( const QStringList & /* ids */ )
{
	//kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotStatusChanged( const QString &who, int stat, const QString &msg, int /* away */)
{
//	kdDebug(14180) << k_funcinfo << endl;
	Kopete::Contact *kc = contact( who );
	if ( kc )
	{
		Kopete::OnlineStatus newStatus = static_cast<YahooProtocol*>( m_protocol )->statusFromYahoo( stat );
		if ( newStatus == static_cast<YahooProtocol*>( m_protocol )->Custom )
			kc->setProperty( m_protocol->awayMessage, msg );
		else
			kc->removeProperty( m_protocol->awayMessage );

		kc->setOnlineStatus( newStatus );
	}

}

void YahooAccount::slotGotIm( const QString &who, const QString &msg, long tm, int /*stat*/)
{
	QFont msgFont;
	QDateTime msgDT;
	Kopete::ContactPtrList justMe;

	if( !contact( who ) )
	{
		kdDebug(14180) << "Adding contact " << who << endl;
		addMetaContact( who,who,  0L, Kopete::Account::Temporary );
	}

    //Parse the message for it's properties
    kdDebug(14180) << "Original message is '" << msg << "'" << endl;
    //kdDebug(14180) << "Message color is " << getMsgColor(msg) << endl;
    QColor fgColor = getMsgColor( msg );
    if (tm == 0)
		msgDT.setTime_t(time(0L));
	else
		msgDT.setTime_t(tm, Qt::LocalTime);

    QString newMsgText = stripMsgColorCodes( msg );

    kdDebug(14180) << "Message after stripping color codes '" << newMsgText << "'" << endl;

   	if (newMsgText.find("<font") != -1)
	{
		msgFont.setFamily(newMsgText.section('"', 1,1));

		if (newMsgText.find("size"))
			msgFont.setPointSize(newMsgText.section('"', 3,3).toInt());

		//remove the font encoding since we handle them ourselves
		newMsgText.remove(newMsgText.mid(0, newMsgText.find('>')+1));
	}

    kdDebug(14180) << "Message after removing font tags '" << newMsgText << "'" << endl;

	Kopete::ChatSession *mm = contact(who)->manager(Kopete::Contact::CanCreate);

	// Tell the message manager that the buddy is done typing
	mm->receivedTypingMsg(contact(who), false);

	justMe.append(myself());

	Kopete::Message kmsg(msgDT, contact(who), justMe, newMsgText,
					Kopete::Message::Inbound , Kopete::Message::RichText);

	kmsg.setFg( fgColor );
	kmsg.setFont(msgFont);
	mm->appendMessage(kmsg);

}

void YahooAccount::slotGotConfInvite( const QString & /* who */, const QString & /* room */, const QString & /* msg */, const QStringList & /* members */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotConfUserDecline( const QString & /* who */, const QString & /* room */, const QString & /* msg */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotConfUserJoin( const QString & /* who */, const QString & /* room */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotConfUserLeave( const QString & /* who */, const QString & /* room */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotConfMessage( const QString & /* who */, const QString & /* room */, const QString & /* msg */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotGotFile( const QString & /* who */, const QString & /* url */, long /* expires */, const QString & /* msg */,
	const QString & /* fname */, unsigned long /* fesize */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotContactAdded( const QString & /* myid */, const QString & /* who */, const QString & /* msg */ )
{
//	kdDebug(14180) << k_funcinfo << myid << " " << who << " " << msg << endl;
}

void YahooAccount::slotRejected( const QString & /* who */, const QString & /* msg */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotTypingNotify( const QString &who, int what )
{
	emit receivedTypingMsg(who, what);
}

void YahooAccount::slotGameNotify( const QString & /* who */, int /* stat */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotMailNotify( const QString& from, const QString& /* subject */, int cnt )
{
//	kdDebug(14180) << k_funcinfo << "Mail count: " << cnt << endl;

	if ( cnt > m_currentMailCount && from.isEmpty() )
	{
		KNotifyClient::event( Kopete::UI::Global::sysTrayWId(), "yahoo_mail",
			i18n( "You have one unread message in your Yahoo inbox.",
			"You have %n unread messages in your Yahoo inbox.", cnt ));
		m_currentMailCount = cnt;
	}
	else if ( cnt > m_currentMailCount )
	{	kdDebug(14180) << k_funcinfo << "attempting to trigger event" << endl;
		KNotifyClient::event( Kopete::UI::Global::sysTrayWId(), "yahoo_mail",
			i18n( "You have a message from %1 in your Yahoo inbox.").arg(from));
		m_currentMailCount = cnt;
	}
}

void YahooAccount::slotSystemMessage( const QString & /* msg */ )
{
//	kdDebug(14180) << k_funcinfo << msg << endl;
}

void YahooAccount::slotError( const QString & err, int fatal )
{
	kdDebug(14180) << k_funcinfo << err << endl;
	m_lastDisconnectCode = fatal;
	if ( fatal == 1 || fatal == 2 )
		disconnect();
}

void YahooAccount::slotRemoveHandler( int /* fd */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::setOnlineStatus( const Kopete::OnlineStatus& status , const QString &reason)
{
	if ( myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Online )
		connect( status );
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Offline )
		disconnect();
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Away )
		slotGoStatus( status.internalStatus(), reason );
}

#include "yahooaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

