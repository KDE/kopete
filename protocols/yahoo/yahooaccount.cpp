/*
    yahooaccount.h - Manages a single Yahoo account

    Copyright (c) 2003 by Gav Wood               <gav@kde.org>
    Based on code by Olivier Goffart             <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

// KDE
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>
#include <kiconloader.h>

// Kopete
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopetemessage.h"
#include "kopeteviewmanager.h"
#include "kopetemessagemanager.h"
#include "kopeteonlinestatus.h"

// Yahoo
#include "yahooaccount.h"
#include "yahooprotocol.h"
#include "yahoocontact.h"

YahooAccount::YahooAccount(YahooProtocol *parent, const QString& AccountID, const char *name)
: KopeteAccount(parent, AccountID, name)
{
	kdDebug(14180) << "YahooAccount::YahooAccount(parent, " << AccountID << ", " << QString(name) << ")" << endl;

	// first things first - initialise internals
	theHaveContactList = false;
	
	// we need this quite early (before initActions at least)
	kdDebug(14180) << "Yahoo: Creating myself with name = " << accountId() << endl;
	m_myself = new YahooContact(this, accountId(), accountId(), 0);
	m_myself->setYahooStatus(YahooStatus::Offline, "", 0);
	
	// now initialise the menu actions
	initActions();
	
	if(autoLogin()) connect();
}

YahooAccount::~YahooAccount()
{
}

void YahooAccount::loaded()
{
	QString publicName = pluginData(protocol(), QString::fromLatin1("displayName"));
	if(!publicName.isNull())
		m_myself->rename(publicName);		//TODO: might cause problems depending on rename semantics
}

YahooSession *YahooAccount::yahooSession()
{
	return m_session ? m_session : 0L;
}

KopeteContact *YahooAccount::myself() const
{
	return m_myself ? m_myself : 0L;
}

void YahooAccount::connect()
{
	YahooSession *session_;
	session_ = 0L;
	
	QString server = static_cast<YahooProtocol *>(protocol())->server();
	int port = static_cast<YahooProtocol *>(protocol())->port();
	// TODO: do something with them! :-)
	
	kdDebug(14180) << "YahooAccount::connect()" << endl;

	if(!isConnected())
	{	kdDebug(14180) << "Attempting to connect to Yahoo. (i would use server <" << server << ":" << port << ">, but i dont know how). user <" << accountId() << ">" << endl;
		session_ = YahooSessionManager::manager()->createSession(accountId(), getPassword(), YAHOO_STATUS_AVAILABLE);
		m_session = session_;
		if(session_)
		{	QTimer::singleShot(5000, this, SLOT(slotGotBuddiesTimeout()));
			m_myself->setYahooStatus(YahooStatus::Available);
			/* We have a session, time to connect its signals to our plugin slots */
			QObject::connect( session_ , SIGNAL(loginResponse( int,  const QString &)), this , SLOT(slotLoginResponse( int, const QString &)) );
			QObject::connect( session_ , SIGNAL(gotBuddy(const QString &, const QString &, const QString &)), this, SLOT(slotGotBuddy(const QString &, const QString &, const QString &)));
			QObject::connect( session_ , SIGNAL(gotIgnore( const QStringList & )), this , SLOT(void slotGotIgnore( const QStringList & )) );
			QObject::connect( session_ , SIGNAL(gotIdentities( const QStringList & )), this , SLOT(slotGotIdentities( const QStringList & )) );
			QObject::connect( session_ , SIGNAL(statusChanged( const QString &, int, const QString &, int)), this , SLOT(slotStatusChanged( const QString &, int , const QString &, int )) );
			QObject::connect( session_ , SIGNAL(gotIm( const QString &, const QString &, long, int )), this , SLOT(slotGotIm( const QString &, const QString &, long, int)) );
			QObject::connect( session_ , SIGNAL(gotConfInvite( const QString &, const QString &, const QString &, const QStringList &)), this , SLOT(slotGotConfInvite( const QString &, const QString &, const QString &, const QStringList &)) );
			QObject::connect( session_ , SIGNAL(confUserDecline( const QString &, const QString &, const QString &)), this , SLOT(slotConfUserDecline( const QString &, const QString &, const QString &)) );
			QObject::connect( session_ , SIGNAL(confUserJoin( const QString &, const QString &)), this , SLOT(slotConfUserJoin( const QString &, const QString &)) );
			QObject::connect( session_ , SIGNAL(confUserLeave( const QString &, const QString &)), this , SLOT(slotConfUserLeave( const QString &, const QString &)) );
			QObject::connect( session_ , SIGNAL(confMessage( const QString &, const QString &, const QString &)), this , SLOT(slotConfMessage( const QString &, const QString &, const QString &)) );
			QObject::connect( session_ , SIGNAL(gotFile( const QString &, const QString &, long, const QString &, const QString &, unsigned long)), this , SLOT(slotGotFile( const QString &, const QString &, long, const QString &, const QString &, unsigned long )) );
			QObject::connect( session_ , SIGNAL(contactAdded( const QString &, const QString &, const QString &)), this , SLOT(slotContactAdded( const QString &, const QString &, const QString &)) );
			QObject::connect( session_ , SIGNAL(rejected( const QString &, const QString &)), this , SLOT(slotRejected( const QString &, const QString &)) );
			QObject::connect( session_ , SIGNAL(typingNotify( const QString &, int)), this , SLOT(slotTypingNotify( const QString &, int )) );
			QObject::connect( session_ , SIGNAL(gameNotify( const QString &, int)), this , SLOT(slotGameNotify( const QString &, int )) );
			QObject::connect( session_ , SIGNAL(mailNotify( const QString &, const QString &, int )), this , SLOT(slotMailNotify( const QString &, const QString &, int )) );
			QObject::connect( session_ , SIGNAL(systemMessage( const QString &)), this , SLOT(slotSystemMessage( const QString &)) );
			QObject::connect( session_ , SIGNAL(error( const QString &, int )), this , SLOT(slotError( const QString &, int )) );
		}
	}
	else if(isAway())
	{	// They're really away, and they want to un-away.
		slotGoOnline();
	}
	else	// Nope, just your regular crack junky.
		kdDebug(14180) << "Yahoo plugin: Ignoring connect request (already connected)." <<endl;
}

void YahooAccount::disconnect()
{
	kdDebug(14180) << "YahooAccount::disconnect()" << endl;

	if(isConnected())
	{	kdDebug(14180) <<  "Attempting to disconnect from Yahoo server " << endl;
		m_session->logOff();
		m_myself->setYahooStatus(YahooStatus::Offline);
		for(QDictIterator<KopeteContact> i(contacts()); i.current(); ++i)
			static_cast<YahooContact *>(i.current())->setYahooStatus(YahooStatus::Offline);
	}
	else	// Again, what's with the crack? Sheez.
		kdDebug(14180) << "Ignoring disconnect request (not connected)." << endl;
}

void YahooAccount::setAway(bool status)
{
	kdDebug(14180) << "YahooAccount::setAway(" << status << ")" << endl;
	// TODO: make it work!
	
}

void YahooAccount::slotConnected()
{
	kdDebug(14180) << "Yahoo: CONNECTED" << endl;
}

void YahooAccount::slotGoOnline()
{
	if(isConnected())
		;// TODO: Do something!
	else
		connect();
}

void YahooAccount::slotGoOffline()
{
	disconnect();
}

void YahooAccount::initActions()
{
	kdDebug(14180) << "YahooAccount::initActions()" << endl;

	// TODO: These need to point to real slots that actually do the job!
	actionGoOnline = new KAction(i18n(YSTAvailable), "yahoo_online",
				0, this, SLOT(connect()), this, "actionYahooConnect");
	actionGoOffline = new KAction(i18n("Offline"), "yahoo_offline",
				0, this, SLOT(disconnect()), this, "actionYahooDisconnect");
/*	actionGoStatus001 = new KAction(i18n(YSTBeRightBack), "yahoo_busy",
				0, this, SLOT(connect()), this, "actionYahooConnect");
	actionGoStatus002 = new KAction(i18n(YSTBusy), "yahoo_busy",
				0, this, SLOT(connect()), this, "actionYahooConnect");
	actionGoStatus003 = new KAction(i18n(YSTNotAtHome), "yahoo_busy",
				0, this, SLOT(connect()), this, "actionYahooConnect");
	actionGoStatus004 = new KAction(i18n(YSTNotAtMyDesk), "yahoo_busy",
				0, this, SLOT(connect()), this, "actionYahooConnect");
	actionGoStatus005 = new KAction(i18n(YSTNotInTheOffice), "yahoo_busy",
				0, this, SLOT(connect()), this, "actionYahooConnect");
	actionGoStatus006 = new KAction(i18n(YSTOnThePhone), "yahoo_busy",
				0, this, SLOT(connect()), this, "actionYahooConnect");
	actionGoStatus007 = new KAction(i18n(YSTOnVacation), "yahoo_busy",
				0, this, SLOT(connect()), this, "actionYahooConnect");
	actionGoStatus008 = new KAction(i18n(YSTOutToLunch), "yahoo_busy",
				0, this, SLOT(connect()), this, "actionYahooConnect");
	actionGoStatus009 = new KAction(i18n(YSTSteppedOut), "yahoo_busy",
				0, this, SLOT(connect()), this, "actionYahooConnect");
	actionGoStatus012 = new KAction(i18n("Invisible"), "yahoo_offline",
				0, this, SLOT(connect()), this, "actionYahooConnect"); // XXX Connect with invisible on
	actionGoStatus099 = new KAction(i18n("Custom"), "yahoo_online",
				0, this, SLOT(connect()), this, "actionYahooConnect"); // XXX Get some dialogbox
	actionGoStatus999 = new KAction(i18n(YSTIdle), "yahoo_idle",
				0, this, SLOT(connect()), this, "actionYahooConnect");
*/
	
	theActionMenu = new KActionMenu("Yahoo", this);
	theActionMenu->popupMenu()->insertTitle(m_myself->icon(), "Yahoo ("+m_myself->displayName()+")");
	theActionMenu->insert(actionGoOnline);
	theActionMenu->insert(actionGoOffline);
/*	theActionMenu->insert(actionGoStatus001);
	theActionMenu->insert(actionGoStatus002);
	theActionMenu->insert(actionGoStatus003);
	theActionMenu->insert(actionGoStatus004);
	theActionMenu->insert(actionGoStatus005);
	theActionMenu->insert(actionGoStatus006);
	theActionMenu->insert(actionGoStatus007);
	theActionMenu->insert(actionGoStatus008);
	theActionMenu->insert(actionGoStatus009);
	theActionMenu->insert(actionGoStatus012);
	theActionMenu->insert(actionGoStatus099);
	theActionMenu->insert(actionGoStatus999);
*/
}

void YahooAccount::slotGotBuddiesTimeout()
{
	slotGotBuddies(yahooSession()->getLegacyBuddyList());
}

void YahooAccount::slotGotBuddies( const YList */*theList*/ )
{
	kdDebug(14180) << "[YahooAccount::slotGotBuddies()]" << endl;
	theHaveContactList = true;
	
	// Serverside -> local
	for(QMap<QString, QPair<QString, QString> >::iterator i = IDs.begin(); i != IDs.end(); i++)
		if(!contacts()[i.key()] && 1/* && importYahooContacts */)		// TODO: make importYahooContacts a configuration option.
		{	kdDebug(14180) << "SS Contact " << i.key() << " is not in the contact list. Adding..." << endl;
			QString groupName = /*importYahooGroups*/ 1 ? i.data().first : QString("Imported Yahoo Contacts");	// TODO: make importYahooGroups a config option.
			addContact(i.key(), i.data().second == "" || i.data().second.isNull() ? i.key() : i.data().second, 0, groupName);
		}
		
	// Local -> serverside
	for(QDictIterator<KopeteContact> i(contacts()); i.current(); ++i)
		if(i.currentKey() != accountId())
			static_cast<YahooContact *>(i.current())->syncToServer();
}

YahooContact *YahooAccount::contact( const QString &id )
{
	return static_cast<YahooContact *>(contacts()[id]);
}

bool YahooAccount::addContactToMetaContact(const QString &contactId, const QString &displayName, KopeteMetaContact *parentContact )
{
	kdDebug(14180) << "[YahooAccount::addContactToMetaContact] contactId: " << contactId << endl;

	if(!contact(contactId))
	{	//No server side stuff for this protocol yet, just create the guy and go
		YahooContact *newContact = new YahooContact( this, contactId, displayName, parentContact);
		return newContact != 0;
	}
	else
		kdDebug(14180) << "[YahooAccount::addContact] Contact already exists" << endl;
	
	return false;
}

/***************************************************************************
 *                                                                         *
 *   Slot for KYahoo signals                                               *
 *                                                                         *
 ***************************************************************************/

void YahooAccount::slotLoginResponse( int /* succ */ , const QString & /* url */ )
{
	kdDebug(14180) << "[YahooAccount::slotLoginResponse]" << endl;
}

void YahooAccount::slotGotBuddy( const QString &userid, const QString &alias, const QString &group )
{
	kdDebug(14180) << "[YahooAccount::slotGotBuddy]" << endl;
	IDs[userid] = QPair<QString, QString>(group, alias);
}
void YahooAccount::slotGotIgnore( YList * /* igns */ )
{
	kdDebug(14180) << "[YahooAccount::slotGotIgnore]" << endl;
}

void YahooAccount::slotGotIdentities( const QStringList & /* ids */ )
{
	kdDebug(14180) << "[YahooAccount::slotGotIdentities]" << endl;
}

void YahooAccount::slotStatusChanged( const QString &who, int stat, const QString &msg, int away)
{
	kdDebug(14180) << "[YahooAccount::slotStatusChanged(" << who << ", " << stat << ", " << msg << ", " << away << "]" << endl;
	if(contact(who))
		contact(who)->setYahooStatus( YahooStatus::fromLibYahoo2(stat), msg, away);
}

void YahooAccount::slotGotIm( const QString &who, const QString &msg, long tm, int stat)
{
	kdDebug(14180) << "[YahooAccount::slotGotIm] " << who << " " << msg << " " << tm << " " << stat << endl;

	if(!contact(who))
		addContact( who, who, 0L, QString::null, true );
	
	KopeteMessageManager *mm = contact(who)->manager();

	// Tell the message manager that the buddy is done typing
	mm->receivedTypingMsg(contact(who), false);

	KopeteContactPtrList justMe;
	justMe.append(myself());
	KopeteMessage kmsg(contact(who), justMe, msg, KopeteMessage::Inbound);
	mm->appendMessage(kmsg);
}

void YahooAccount::slotGotConfInvite( const QString & /* who */, const QString & /* room */, const QString & /* msg */, const QStringList & /* members */ )
{
	kdDebug(14180) << "[YahooAccount::slotGotConfInvite]" << endl;
}

void YahooAccount::slotConfUserDecline( const QString & /* who */, const QString & /* room */, const QString & /* msg */ )
{
	kdDebug(14180) << "[YahooAccount::slotConfUserDecline]" << endl;
}

void YahooAccount::slotConfUserJoin( const QString & /* who */, const QString & /* room */ )
{
	kdDebug(14180) << "[YahooAccount::slotConfUserJoin]" << endl;
}

void YahooAccount::slotConfUserLeave( const QString & /* who */, const QString & /* room */ )
{
	kdDebug(14180) << "[YahooAccount::slotConfUserLeave]" << endl;
}

void YahooAccount::slotConfMessage( const QString & /* who */, const QString & /* room */, const QString & /* msg */ )
{
	kdDebug(14180) << "[YahooAccount::slotConfMessage]" << endl;
}

void YahooAccount::slotGotFile( const QString & /* who */, const QString & /* url */, long /* expires */, const QString & /* msg */,
	const QString & /* fname */, unsigned long /* fesize */ )
{
	kdDebug(14180) << "[YahooAccount::slotGotFile]" << endl;
}

void YahooAccount::slotContactAdded( const QString &  myid , const QString &  who , const QString &  msg  )
{
	kdDebug(14180) << "[YahooAccount::slotContactAdded] " << myid << " " << who << " " << msg << endl;
}

void YahooAccount::slotRejected( const QString & /* who */, const QString & /* msg */ )
{
	kdDebug(14180) << "[YahooAccount::slotRejected]" << endl;
}

void YahooAccount::slotTypingNotify( const QString &who, int what )
{
	kdDebug(14180) << "[YahooAccount::slotTypingNotify] " << who << " " << what << endl;
}

void YahooAccount::slotGameNotify( const QString & /* who */, int /* stat */ )
{
	kdDebug(14180) << "[YahooAccount::slotGameNotify]" << endl;
}

void YahooAccount::slotMailNotify( const QString & /* from */, const QString & /* subj */, int /* cnt */)
{
	kdDebug(14180) << "[YahooAccount::slotMailNotify]" << endl;
}

void YahooAccount::slotSystemMessage( const QString &msg )
{
	kdDebug(14180) << "[YahooAccount::slotSystemMessage]" << msg << endl;
}

void YahooAccount::slotError( const QString & /* err */, int /* fatal */ )
{
	kdDebug(14180) << "[YahooAccount::slotError]" << endl;
}

void YahooAccount::slotRemoveHandler( int /* fd */ )
{
	kdDebug(14180) << "[YahooAccount::slotRemoveHandler]" << endl;
}

#include "yahooaccount.moc"


