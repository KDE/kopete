/*
    yahooaccount.h - Manages a single Yahoo account

    Copyright (c) 2003 by Gav Wood               <gav@kde.org>
    Copyright (c) 2003 by Matt Rogers            <mattrogers@sbcglobal.net>
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
//QT
#include <qfont.h>

// KDE
#include <kconfig.h>
#include <kdebug.h>
#include <kaction.h>
#include <kpopupmenu.h>

// Kopete
#include "kopetemessagemanager.h"

// Yahoo
#include "yahooaccount.h"
#include "yahoocontact.h"

YahooAccount::YahooAccount(YahooProtocol *parent, const QString& AccountID, const char *name)
: KopeteAccount(parent, AccountID, name)
{
//	kdDebug(14180) << k_funcinfo << AccountID << ", " << QString(name) << ")" << endl;

	// first things first - initialise internals
	theHaveContactList = false;
	stateOnConnection = 0;
	theAwayDialog = 0L;

	// we need this quite early (before initActions at least)
	kdDebug(14180) << "Yahoo: Creating myself with name = " << accountId() << endl;
	m_myself = new YahooContact(this, accountId(), accountId(), 0);
	m_myself->setYahooStatus(YahooStatus::Offline, "", 0);

	if(autoLogin()) connect();
}

YahooAccount::~YahooAccount()
{
}

void YahooAccount::slotGoStatus(int status, const QString &awayMessage)
{
	kdDebug(14180) << k_funcinfo << status << awayMessage << endl;

	if(!isConnected())
	{	if(status == 12)
			// TODO: must connect as invisible
			connect();
		else
			connect();
		stateOnConnection = status;
	}
	else
	{	m_session->setAway(yahoo_status(status), awayMessage, status ? 1 : 0);
		m_myself->setYahooStatus(YahooStatus::fromLibYahoo2(status));
	}
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
	YahooSession *session_ = 0;

	QString server = static_cast<YahooProtocol *>(protocol())->server();
	int port = static_cast<YahooProtocol *>(protocol())->port();
//	kdDebug(14180) << k_funcinfo << endl;

	if(!isConnected())
	{	kdDebug(14180) << "Attempting to connect to Yahoo on <" << server << ":" << port << ">. user <" << accountId() << ">" << endl;
		YahooSessionManager::manager()->setPager(server, port);
		session_ = YahooSessionManager::manager()->createSession(accountId(), getPassword(), YAHOO_STATUS_AVAILABLE);
		m_session = session_;
		if(session_ > 0)
		{	if(session_->Id() > 0)
			{
				kdDebug(14180) << "We appear to have connected (session: " << session_ << endl;
				m_myself->setYahooStatus(YahooStatus::Available);
				/* We have a session, time to connect its signals to our plugin slots */
				QObject::connect( session_ , SIGNAL(loginResponse( int,  const QString &)), this , SLOT(slotLoginResponse( int, const QString &)) );
				QObject::connect( session_ , SIGNAL(gotBuddy(const QString &, const QString &, const QString &)), this, SLOT(slotGotBuddy(const QString &, const QString &, const QString &)));
				QObject::connect( session_ , SIGNAL(gotIgnore( const QStringList & )), this , SLOT(void slotGotIgnore( const QStringList & )) );
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
				QObject::connect( session_ , SIGNAL(gotIdentities( const QStringList & )), this , SLOT(slotGotIdentities( const QStringList & )) );
			}
			else
			{
				kdDebug(14180) << "Couldn't connect!" << endl;
				delete session_;
				// TODO: message box saying can't connect?
			}
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
//	kdDebug(14180) << k_funcinfo << endl;

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

void YahooAccount::setAway(bool status, const QString &awayMessage)
{
	kdDebug(14180) << "YahooAccount::setAway(" << status << ", " << awayMessage << ")" << endl;
	if(awayMessage.isEmpty())
		slotGoStatus(status ? 2 : 0);
	else
		slotGoStatus(status ? 99 : 0, awayMessage);
}

void YahooAccount::slotConnected()
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotGoOnline()
{
	if(!isConnected())
		connect();
	else
		slotGoStatus(0);
}

void YahooAccount::slotGoOffline()
{
	if(isConnected())
		disconnect();
	else
		m_myself->setYahooStatus(YahooStatus::Offline);
}

KActionMenu *YahooAccount::actionMenu()
{
//	kdDebug(14180) << k_funcinfo << endl;

	KActionMenu *theActionMenu = new KActionMenu("Yahoo", this);
	theActionMenu->popupMenu()->insertTitle(m_myself->icon(), "Yahoo ("+m_myself->displayName()+")");

	theActionMenu->insert(new KAction(i18n(YSTAvailable), YahooStatus(YahooStatus::Available).ys2kos().iconFor(this), 0, this, SLOT(slotGoOnline()), this, "actionYahooGoOnline"));
	theActionMenu->insert(new KAction(i18n(YSTBeRightBack), YahooStatus(YahooStatus::BeRightBack).ys2kos().iconFor(this), 0, this, SLOT(slotGoStatus001()), this, "actionYahooGoStatus001"));
	theActionMenu->insert(new KAction(i18n(YSTBusy), YahooStatus(YahooStatus::Busy).ys2kos().iconFor(this), 0, this, SLOT(slotGoStatus002()), this, "actionYahooGoStatus002"));
	theActionMenu->insert(new KAction(i18n(YSTNotAtHome), YahooStatus(YahooStatus::NotAtHome).ys2kos().iconFor(this), 0, this, SLOT(slotGoStatus003()), this, "actionYahooGoStatus003"));
	theActionMenu->insert(new KAction(i18n(YSTNotAtMyDesk), YahooStatus(YahooStatus::NotAtMyDesk).ys2kos().iconFor(this), 0, this, SLOT(slotGoStatus004()), this, "actionYahooGoStatus004"));
	theActionMenu->insert(new KAction(i18n(YSTNotInTheOffice), YahooStatus(YahooStatus::NotInTheOffice).ys2kos().iconFor(this), 0, this, SLOT(slotGoStatus005()), this, "actionYahooGoStatus005"));
	theActionMenu->insert(new KAction(i18n(YSTOnThePhone), YahooStatus(YahooStatus::OnThePhone).ys2kos().iconFor(this), 0, this, SLOT(slotGoStatus006()), this, "actionYahooGoStatus006"));
	theActionMenu->insert(new KAction(i18n(YSTOnVacation), YahooStatus(YahooStatus::OnVacation).ys2kos().iconFor(this), 0, this, SLOT(slotGoStatus007()), this, "actionYahooGoStatus007"));
	theActionMenu->insert(new KAction(i18n(YSTOutToLunch), YahooStatus(YahooStatus::OutToLunch).ys2kos().iconFor(this), 0, this, SLOT(slotGoStatus008()), this, "actionYahooGoStatus008"));
	theActionMenu->insert(new KAction(i18n(YSTSteppedOut), YahooStatus(YahooStatus::SteppedOut).ys2kos().iconFor(this), 0, this, SLOT(slotGoStatus009()), this, "actionYahooGoStatus009"));
	theActionMenu->insert(new KAction(i18n(YSTInvisible), YahooStatus(YahooStatus::Invisible).ys2kos().iconFor(this), 0, this, SLOT(slotGoStatus012()), this, "actionYahooGoStatus012"));
	theActionMenu->insert(new KAction(i18n("Custom"), YahooStatus(YahooStatus::Custom).ys2kos().iconFor(this), 0, this, SLOT(slotGoStatus099()), this, "actionYahooGoStatus099"));
	theActionMenu->insert(new KAction(i18n(YSTOffline), YahooStatus(YahooStatus::Offline).ys2kos().iconFor(this), 0, this, SLOT(slotGoOffline()), this, "actionYahooGoOffline"));
	// TODO: do something(?!) with it
//	theActionMenu->insert(new KAction(i18n(YSTIdle), YahooStatus(YahooStatus::Idle).ys2kos().iconFor(this), 0, this, SLOT(slotGoStatus999()), this, "actionYahooGoStatus999");

	return theActionMenu;
}

void YahooAccount::slotGotBuddies( const YList */*theList*/ )
{
//	kdDebug(14180) << k_funcinfo << endl;
	theHaveContactList = true;
	KGlobal::config()->setGroup("Yahoo");
	YahooProtocol *theProtocol = static_cast<YahooProtocol *>(protocol());

	// Serverside -> local
	for(QMap<QString, QPair<QString, QString> >::iterator i = IDs.begin(); i != IDs.end(); i++)
		if(!contacts()[i.key()] && theProtocol->importContacts())
		{	kdDebug(14180) << "SS Contact " << i.key() << " is not in the contact list. Adding..." << endl;
			QString groupName = theProtocol->useGroupNames() ? i.data().first : QString("Imported Yahoo Contacts");
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
//	kdDebug(14180) << k_funcinfo << " contactId: " << contactId << endl;

	if(!contact(contactId))
	{	YahooContact *newContact = new YahooContact( this, contactId, displayName, parentContact);
		return newContact != 0;
	}
	else
		kdDebug(14180) << k_funcinfo << "Contact already exists" << endl;

	return false;
}


/***************************************************************************
 *                                                                         *
 *   Slot for KYahoo signals                                               *
 *                                                                         *
 ***************************************************************************/

void YahooAccount::slotLoginResponse( int succ , const QString &url )
{
	kdDebug(14180) << k_funcinfo << succ << ", " << url << ")]" << endl;
	slotGotBuddies(yahooSession()->getLegacyBuddyList());
	if(stateOnConnection)
	{	m_session->setAway(yahoo_status(stateOnConnection), "", 0);
		m_myself->setYahooStatus(YahooStatus::fromLibYahoo2(stateOnConnection));
		stateOnConnection = 0;
	}
}

void YahooAccount::slotGotBuddy( const QString &userid, const QString &alias, const QString &group )
{
	kdDebug(14180) << k_funcinfo << endl;
	IDs[userid] = QPair<QString, QString>(group, alias);
}

void YahooAccount::slotGotIgnore( const QStringList & /* igns */ )
{
	//kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotGotIdentities( const QStringList & /* ids */ )
{
	//kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotStatusChanged( const QString &who, int stat, const QString &msg, int away)
{
	kdDebug(14180) << k_funcinfo << who << ", " << stat << ", " << msg << ", " << away << "]" << endl;
	if(contact(who))
		contact(who)->setYahooStatus( YahooStatus::fromLibYahoo2(stat), msg, away);
}

void YahooAccount::slotGotIm( const QString &who, const QString &msg, long /*tm*/, int /*stat*/)
{
	QFont msgFont;

	if(!contact(who))
		addContact( who, who, 0L, QString::null, true );

	KopeteMessageManager *mm = contact(who)->manager();

	// Tell the message manager that the buddy is done typing
	mm->receivedTypingMsg(contact(who), false);

	KopeteContactPtrList justMe;
	justMe.append(myself());
	KopeteMessage kmsg(contact(who), justMe, msg, KopeteMessage::Inbound , KopeteMessage::PlainText);
	QString newMsg = kmsg.plainBody();

	if (newMsg.find("<font") != -1)
	{
		msgFont.setFamily(newMsg.section('"', 1,1));

		if (newMsg.find("size"))
			msgFont.setPointSize(newMsg.section('"', 3,3).toInt());

		//remove the font encoding since we handle them ourselves
		newMsg.remove(newMsg.mid(0, newMsg.find('>')+1));
	}
	//set the new body that has correct HTML
	kmsg.setBody(newMsg, KopeteMessage::RichText);
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

void YahooAccount::slotContactAdded( const QString &  myid , const QString &  who , const QString &  msg  )
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

void YahooAccount::slotMailNotify( const QString & /* from */, const QString & /* subj */, int /* cnt */)
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotSystemMessage( const QString &msg )
{
//	kdDebug(14180) << k_funcinfo << msg << endl;
}

void YahooAccount::slotError( const QString & /* err */, int /* fatal */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotRemoveHandler( int /* fd */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAwayDialog::setAway(int awayType)
{
	theAccount->setAway(awayType, getSelectedAwayMessage());
}


#include "yahooaccount.moc"


