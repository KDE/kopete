/*
    oscarcontact.cpp  -  Oscar Protocol Plugin

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "oscarcontact.h"

#include <time.h>

#include <qapplication.h>
#include <qregexp.h>
#include <qstylesheet.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kfiledialog.h>

#include "aim.h"
#include "kopeteaway.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"
#include "kopetecontactlist.h"
#include "kopetegroup.h"
#include "oscarprotocol.h"
#include "oscarsocket.h"
#include "oscaruserinfo.h"

OscarContact::OscarContact(const QString name, OscarProtocol *protocol,
		KopeteMetaContact *parent)
: KopeteContact(protocol, name, parent)
{
	kdDebug(14150) << "[OscarContact] OscarContact(), name=" << name << endl;

	mName = name;
	mProtocol = protocol;
	mMsgManager = 0L;
	mListContact = mProtocol->buddyList()->findBuddy(mName);
	mIdle = 0;
	mLastAutoResponseTime = 0;
	setFileCapable(true);
	mStatus = -1; //OSCAR_OFFLINE;

	if (!mListContact)
	{
		kdDebug(14150) << "[OSCAR] ERROR, mListContact is NULL! Ehem, why?" << endl;
		mListContact = new AIMBuddy(mProtocol->randomNewBuddyNum, 0, mName);
		mProtocol->randomNewBuddyNum++;
		mProtocol->buddyList()->addBuddy(mListContact);
		if (!mListContact)
			kdDebug(14150) << "[OSCAR] ERROR, mListContact is *STILL* NULL! Prepare to crashz0r!" << endl;
	}

	// Buddy Changed
	QObject::connect(mProtocol->engine, SIGNAL(gotBuddyChange(UserInfo)),
					this,SLOT(slotBuddyChanged(UserInfo)));
	// Buddy offline
	QObject::connect(mProtocol->engine, SIGNAL(gotOffgoingBuddy(QString)),
					this,SLOT(slotOffgoingBuddy(QString)));
	// Got IM
	QObject::connect(mProtocol->engine, SIGNAL(gotIM(QString,QString,bool)),
					this,SLOT(slotIMReceived(QString,QString,bool)));
	// User's status changed (I don't understand this)
	QObject::connect(mProtocol->engine, SIGNAL(statusChanged(int)),
					this, SLOT(slotMainStatusChanged(int)));
	// Incoming minitype notification
	QObject::connect(mProtocol->engine, SIGNAL(gotMiniTypeNotification(QString, int)),
					this, SLOT(slotGotMiniType(QString, int)));
	// New direct connection
	QObject::connect(mProtocol->engine, SIGNAL(connectionReady(QString)),
		this, SLOT(slotDirectIMReady(QString)));
	// Direct connection closed
	QObject::connect(mProtocol->engine, SIGNAL(directIMConnectionClosed(QString)),
		this, SLOT(slotDirectIMConnectionClosed(QString)));
	//File transfer request
	QObject::connect(mProtocol->engine, SIGNAL(gotFileSendRequest(QString,QString,QString,unsigned long)),
		this, SLOT(slotGotFileSendRequest(QString,QString,QString,unsigned long)));
	//File transfer started
	QObject::connect(mProtocol->engine, SIGNAL(transferBegun(OscarConnection *, const QString &, const unsigned long, const QString &)),
		this, SLOT(slotTransferBegun(OscarConnection *, const QString &, const unsigned long, const QString &)));
	//File transfer manager stuff
	QObject::connect( KopeteTransferManager::transferManager(), SIGNAL(accepted(KopeteTransfer *, const QString &)),
		this, SLOT(slotTransferAccepted(KopeteTransfer *, const QString &)) );
	QObject::connect( KopeteTransferManager::transferManager(), SIGNAL(refused(const KopeteFileTransferInfo &)),
		this, SLOT(slotTransferDenied(const KopeteFileTransferInfo &)) );
	// When the contact is being removed (whether from a group or not)
	QObject::connect((KopeteContact *)(this), SIGNAL(contactDestroyed( KopeteContact *c )), this, SLOT(slotContactDestroyed( KopeteContact *c )));
	QObject::connect(KopeteContactList::contactList(), SIGNAL(groupRemoved( KopeteGroup * )), this, SLOT(slotGroupRemoved( KopeteGroup * )));
	initActions();

	if ( !mListContact->alias().isEmpty() )
		setDisplayName( mListContact->alias() );
	else
		setDisplayName( mListContact->screenname() );

		slotUpdateBuddy();

	theContacts.append( this );
}

OscarContact::~OscarContact()
{
	kdDebug(14150) << "[OscarContact] ~OscarContact()" << endl;
}

/** Return the protocol specific serialized data that a plugin may want to store a contact list. */
QString OscarContact::data(void) const
{
	AIMBuddy *mListContact = mProtocol->buddyList()->findBuddy(mName);

	if (!mListContact)
		if (mListContact->alias())
			return mListContact->alias();
	return QString::null;
}

KopeteMessageManager* OscarContact::manager( bool )
{
	if ( mMsgManager )
		return mMsgManager;
	else
	{
		//printf("Creating a mmsgmanager: %d\n",mProtocol->myself());fflush(stdout);
		mMsgManager =
				KopeteMessageManagerFactory::factory()->create(
								mProtocol->myself(), theContacts, mProtocol);
		QObject::connect(
					mMsgManager,
					SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager *)),
					this,
					SLOT(slotSendMsg(KopeteMessage&, KopeteMessageManager *)));
		QObject::connect(
					mMsgManager,
					SIGNAL(destroyed()),
					this,
					SLOT(slotMessageManagerDestroyed()));
		QObject::connect(
					mMsgManager,
					SIGNAL(typingMsg(bool)),
					this,
					SLOT(slotTyping(bool)));

		return mMsgManager;
	}
}

void OscarContact::slotMessageManagerDestroyed()
{
	mMsgManager = 0L;
}

void OscarContact::slotMainStatusChanged(int newStatus)
{
	if (newStatus == OSCAR_OFFLINE)
	{
		mStatus = OSCAR_OFFLINE;
		setOnlineStatus( Offline );
		// Try to do this, otherwise no big deal
		AIMBuddy *mListContact = mProtocol->buddyList()->findBuddy(mName);
		if ( !mListContact)
			mListContact->setStatus(OSCAR_OFFLINE);
	}
}

/** Called when a buddy changes */
void OscarContact::slotUpdateBuddy()
{
	// status did not change, do nothing
	if ( ( mStatus == mListContact->status() ) && ( mIdle == mListContact->idleTime() ) )
		return;

	// if we have become idle
	if ( mProtocol->isConnected() )
	{
		if ( mListContact->idleTime() > 0 )
		{
			kdDebug(14150) << "[OscarContact] setting " << mName << " idle! Idletime: " << mListContact->idleTime() << endl;
			setIdleState(Idle);
		}
		// we have become un-idle
		else if ( mProtocol->isConnected() )
		{
			kdDebug(14150) << "[OscarContact] setting " << mName << " active!" << endl;
			setIdleState(Active);
		}
		mIdle = mListContact->idleTime();
	}
	mStatus = mListContact->status();
	kdDebug(14150) << "[OscarContact] slotUpdateBuddy(), Contact " << mName << " is now " << mStatus << endl;

	if ( mProtocol->isConnected() ) // oscar-plugin is online
	{
		if ( mName != mListContact->screenname() ) // contact changed his nickname
		{
			if ( !mListContact->alias().isEmpty() )
				setDisplayName(mListContact->alias());
			else
				setDisplayName(mListContact->screenname());
		}
	}
	else // oscar-plugin is offline so all users are offline too
	{
		mStatus = OSCAR_OFFLINE;
		mListContact->setStatus(OSCAR_OFFLINE);

		setOnlineStatus( Offline );
		return;
	}

	// We can only send messages to online user
	if( mStatus == OSCAR_ONLINE )
		setOnlineStatus( Online );
	else if( mStatus == OSCAR_AWAY )
		setOnlineStatus( Away );
	else
		setOnlineStatus( Offline );
}

/** Initialzes the actions */
void OscarContact::initActions(void)
{
	actionCollection = 0L;

	actionWarn = new KAction(i18n("&Warn"), 0, this, SLOT(slotWarn()), this, "actionWarn");
	actionBlock = new KAction(i18n("&Block"), 0, this, SLOT(slotBlock()), this, "actionBlock");
	actionDirectConnect = new KAction(i18n("&Direct IM"), 0, this, SLOT(slotDirectConnect()), this, "actionDirectConnect");
}

/** Returns the status icon of the contact */
QString OscarContact::statusIcon(void) const
{
	if (mStatus == OSCAR_ONLINE)
		return "oscar_online";
	else if (mStatus == OSCAR_AWAY)
		return "oscar_away";
	else
		return "oscar_offline";
}

/** Called when a buddy is oncoming */
void OscarContact::slotBuddyChanged(UserInfo u)
{
	if (tocNormalize(u.sn) == tocNormalize(mName))
	//if we are the contact that is oncoming
	{
		kdDebug(14150) << "[OscarContact] Setting status for " << u.sn << endl;
		if ( u.userclass & USERCLASS_AWAY )
			mListContact->setStatus(OSCAR_AWAY);
		else
			mListContact->setStatus(OSCAR_ONLINE);
		mListContact->setEvil(u.evil);
		mListContact->setIdleTime(u.idletime);
		mListContact->setSignOnTime(u.onlinesince);
		slotUpdateBuddy();
	}
}

// Called when we get a minityping notification
void OscarContact::slotGotMiniType(QString screenName, int type)
{
	//TODO
	// Check to see if it's us
	//kdDebug(14150) << "[OSCAR] Minitype: Comparing "
	//					<< tocNormalize(screenName) << " and "
	//					<< tocNormalize(mName) << endl;

	if( tocNormalize( screenName ) != tocNormalize( mName ) )
		return;

	kdDebug( 14150 ) << k_funcinfo << "Got minitype notification for " << mName << endl;

	// If we already have a message manager
	if( mMsgManager )
	{
		if( type == 2 )
			mMsgManager->receivedTypingMsg( this, true );
		else
			mMsgManager->receivedTypingMsg( this, false );
	}
}

// Called when we want to send a typing notification to
// the other person
void OscarContact::slotTyping( bool typing )
{
	kdDebug( 14150 ) << k_funcinfo << "Typing: " << typing << endl;

	mProtocol->engine->sendMiniTypingNotify( tocNormalize( mName ),
		typing ? OscarSocket::TypingBegun : OscarSocket::TypingFinished );
}

/** Called when a buddy is offgoing */
void OscarContact::slotOffgoingBuddy(QString sn)
{
	if (tocNormalize(sn) == tocNormalize(mName))
	//if we are the contact that is offgoing
	{
		mListContact->setStatus(OSCAR_OFFLINE);
		slotUpdateBuddy();
	}
}

/** Called when user info is requested */
void OscarContact::slotUserInfo(void)
{
	if (!mProtocol->isConnected())
		KMessageBox::sorry(qApp->mainWidget(),
			i18n("<qt>Sorry, you must be connected to the AIM server to retrieve user information, but you will be allowed to continue if you	would like to change the user's nickname.</qt>"),
			i18n("You Must be Connected") );
	else
		if (mListContact->status() == TAIM_OFFLINE)
		{
			KMessageBox::sorry(qApp->mainWidget(),
				i18n("<qt>Sorry, this user isn't online for you to view his/her information, but you will be allowed to only change his/her nickname. Please wait until this user becomes available and try again</qt>" ),
				i18n("User not Online"));
		}

	OscarUserInfo *Oscaruserinfo =
		new OscarUserInfo(mName, mListContact->alias(), mProtocol, *mListContact);

	connect(Oscaruserinfo, SIGNAL(updateNickname(const QString)),
		this, SLOT(slotUpdateNickname(const QString)));

	Oscaruserinfo->show();
}

// Called when an IM is received
void OscarContact::slotIMReceived(QString message, QString sender, bool /*isAuto*/)
{
	// Check if we're the one who sent the message
	if ( tocNormalize(sender) != tocNormalize(mName) )
		return;

	// Tell the message manager that the buddy is done typing
	manager()->receivedTypingMsg( this, false );

	// Build a KopeteMessage and set the body as Rich Text
	KopeteContactPtrList tmpList;
	tmpList.append(mProtocol->myself());
	KopeteMessage msg = parseAIMHTML( message );
	manager()->appendMessage(msg);

	// send our away message in fire-and-forget-mode :)
	if ( mProtocol->isAway() )
	{
		// Get the current time
		long currentTime = time(0L);

		// Compare to the last time we sent a message
		// We'll wait 2 minutes between responses
		if( (currentTime - mLastAutoResponseTime) > 120 )
		{
			kdDebug(14150) << "[OscarContact] slotIMReceived() while we are away, sending away-message to annoy buddy :)" << endl;
			// Send the autoresponse
			mProtocol->engine->sendIM(
					KopeteAway::getInstance()->message(),
					mName, true);
			// Build a pointerlist to insert this contact into
			KopeteContactPtrList toContact;
			toContact.append(this);
			// Display the autoresponse
			// Make it look different
			QString responseDisplay = KopeteAway::getInstance()->message();
			responseDisplay.prepend("<font color='#666699'>Autoresponse: </font>");

			KopeteMessage message( mProtocol->myself(), toContact,
					responseDisplay,
					KopeteMessage::Outbound,
					KopeteMessage::RichText);

			manager()->appendMessage(message);

			// Set the time we last sent an autoresponse
			// which is right now
			mLastAutoResponseTime = time(0L);
		}
	}
}

/** Called when we want to send a message */
void OscarContact::slotSendMsg(KopeteMessage& message, KopeteMessageManager *)
{
	if ( message.body().isEmpty() ) // no text, do nothing
		return;

	// Check to see if we're even online
	if (!mProtocol->isConnected())
	{
		KMessageBox::sorry(qApp->mainWidget(),
			i18n("<qt>You must be logged on to AIM before you can send a message to a user.</qt>"),
			i18n("Not Signed On"));
		return;
	}

	// Check to see if the person we're sending the message to is online
	if (mListContact->status() == TAIM_OFFLINE || mStatus == TAIM_OFFLINE)
	{
			KMessageBox::sorry(qApp->mainWidget(),
							i18n("<qt>This user is not online at the moment for you to message him/her. AIM users must be online for you to be able to message them.</qt>"),
							i18n("User not Online"));
			return;
	}

	mProtocol->engine->sendIM(message.escapedBody(), mName, false);

	// Show the message we just sent in the chat window
	manager()->appendMessage(message);

	manager()->messageSucceeded();
}

/** Called when nickname needs to be updated */
void OscarContact::slotUpdateNickname(const QString newNickname)
{
	setDisplayName( newNickname );
	//emit updateNickname ( newNickname );

	mListContact->setAlias(newNickname);
}

/** Return whether or not this contact is REACHABLE. */
bool OscarContact::isReachable(void)
{
	return (mStatus != OSCAR_OFFLINE);
}

/** Returns a set of custom menu items for the context menu */
KActionCollection *OscarContact::customContextMenuActions(void)
{
	if( actionCollection != 0L )
		delete actionCollection;

	actionCollection = new KActionCollection(this);
	actionCollection->insert( actionWarn );
	actionCollection->insert( actionBlock );
	//experimental
	actionCollection->insert( actionDirectConnect );
	return actionCollection;
}

/** Method to delete a contact from the contact list */
void OscarContact::slotDeleteContact(void)
{
	AIMGroup *group = mProtocol->buddyList()->findGroup(mListContact->groupID());
	if (!group) return;
	mProtocol->buddyList()->removeBuddy(mListContact);
	mProtocol->engine->sendDelBuddy(mListContact->screenname(),group->name());
	deleteLater();
}

void OscarContact::slotContactDestroyed( KopeteContact *c )
{
	slotDeleteContact();
}

void OscarContact::slotGroupRemoved( KopeteGroup *group )
{
	kdDebug(14150) << "[OscarContact] slotGroupRemoved() being called" << endl;
	QString groupName=group->displayName();
	AIMGroup *aGroup = mProtocol->buddyList()->findGroup(mListContact->groupID());
	if (!aGroup) return;
	if (aGroup->name() != groupName) return;
	kdDebug(14150) << "[OscarContact] slotGroupRemoved() calling slotDeleteContact()" << endl;
	slotDeleteContact();
}

void OscarContact::slotWarn()
{
	QString message = i18n( "<qt>Would you like to warn %1 anonymously?" \
	" Select \"Yes\" to warn anonymously, \"No\" to warn" \
	" the user showing them your name, or \"Cancel\" to abort" \
	" warning. (Warning a user on AIM will result in a \"Warning Level\"" \
	" increasing for the user you warn. Once this level has reached a" \
	" certain point, they will not be able to sign on. Please do not abuse" \
	" this function, it is meant for legitimate practices.)</qt>" ).arg(mName);
	QString title = i18n("Warn User %1 anonymously?").arg(mName);

	int result = KMessageBox::questionYesNoCancel(qApp->mainWidget(), message, title);
	if (result == KMessageBox::Yes)
		mProtocol->engine->sendWarning(mName, true);
	else if (result == KMessageBox::No)
		mProtocol->engine->sendWarning(mName, false);
}



KopeteMessage OscarContact::parseAIMHTML ( QString m )
{
/*	============================================================================================
	Original AIM-Messages, just a few to get the idea of the weird format[tm]:

	From original AIM:
	<HTML><BODY BGCOLOR="#ffffff"><FONT FACE="Verdana" SIZE=4>some text message</FONT></BODY></HTML>

	From Trillian 0.7something:
	<HTML><BODY BGCOLOR="#ffffff"><font face="Arial"><b>bin ich ueberhaupt ein standard?</b></BODY></HTML>
	<HTML><BODY BGCOLOR="#ffffff"><font face="Arial"><font color="#ffff00">ups</BODY></HTML>
	<HTML><BODY BGCOLOR="#ffffff"><font face="Arial"><font back="#00ff00">bggruen</BODY></HTML>
	<HTML><BODY BGCOLOR="#ffffff"><font face="Arial"><font back="#00ff00"><font color="#ffff00">both</BODY></HTML>
	<HTML><BODY BGCOLOR="#ffffff"><font face="Arial">LOL</BODY></HTML>

	From GAIM:
	<FONT COLOR="#0002A6"><FONT SIZE="2">cool cool</FONT></FONT>
	============================================================================================ */

	kdDebug(14150) << "AIM Plugin: original message: " << m << endl;

	// This code relies on QT 3.1, when we support that,
	// put it back in
//	QRegExp expr;
//	expr.setCaseSensitive( false );
//	expr.setWildcard( true );
//	expr.setMinimal( true );

	QString result = m;
//	expr.setPattern( "^<html.*>" );
//	result.remove( expr );
//	expr.setPattern( "^<body.*>" );
//	result.remove( expr );
//	expr.setPattern( "</html>$" );
//	result.remove( expr );
//	expr.setPattern( "</body>$" );
//	result.remove( expr );

	bool removeMoreTags = true;
	int pos;
	while(removeMoreTags){
			// If there are more beginning html tags
			pos = result.find("<html>",0,false);
			if(pos != -1){
					result.remove(pos, 6);
			}

			// If there are more ending html tags
			pos = result.find("</html>",0,false);
			if(pos != -1){
					result.remove(pos, 7);
			}

			// If there are more beginning body tags
			pos = result.find("<body>",0,false);
			if(pos != -1){
					result.remove(pos, 6);
			}

			// If there are more ending body tags
			pos = result.find("</body>",0,false);
			if(pos != -1){
					result.remove(pos, 7);
			}

			// Check if there are more tags to remove
			removeMoreTags = false;
			if((result.find("<html>",0,false) > -1) ||
							(result.find("</html>",0,false) > -1) ||
							(result.find("<body>",0,false) > -1) ||
							(result.find("</body>",0,false) > -1)){
					removeMoreTags = true;
			}
	}

	KopeteContactPtrList tmpList;
	tmpList.append(mProtocol->myself());
	KopeteMessage msg( this, tmpList, result, KopeteMessage::Inbound, KopeteMessage::RichText);

	// We don't actually do anything in there yet, but we might eventually

	return msg;
}

/** Called when we want to block the contact */
void OscarContact::slotBlock(void)
{
	QString message = i18n( "<qt>Are you sure you want to block %1? \
	Once blocked, this user will no longer be visible to you. The block can be \
	removed later in the preferences dialog.</qt>" ).arg(mName);
	QString title = i18n("Block User %1?").arg(mName);

	int result = KMessageBox::questionYesNo(qApp->mainWidget(), message, title);
	if (result == KMessageBox::Yes)
	{
		mProtocol->engine->sendBlock(mName);
	}
}

/** Called when we want to connect directly to this contact */
void OscarContact::slotDirectConnect(void)
{
	kdDebug(14150) << "[OscarContact] Requesting direct IM with " << mName << endl;
	QString message = i18n( "<qt>Are you sure you want to establish a direct connection to %1? \
	This will allow %2 to know your IP address, which can be dangerous if you do not trust this contact</qt>" ).arg(mName).arg(mName);
	QString title = i18n("Request Direct IM with %1?").arg(mName);
	int result = KMessageBox::questionYesNo(qApp->mainWidget(), message, title);
	if ( result == KMessageBox::Yes )
	{
		execute();
		KopeteContactPtrList p;
		p.append(this);
		KopeteMessage msg = KopeteMessage(this, p, i18n("Waiting for %1 to connect...").arg(mName), KopeteMessage::Internal, KopeteMessage::PlainText );
		manager()->appendMessage(msg);
		mProtocol->engine->sendDirectIMRequest(mName);
	}
}

/** Called when we become directly connected to the contact */
void OscarContact::slotDirectIMReady(QString name)
{
	// Check if we're the one who is directly connected
	if ( tocNormalize(name) != tocNormalize(mName) )
		return;

	kdDebug(14150) << "[OscarContact] Setting direct connect state for " << mName << " to true." << endl;
	mDirectlyConnected = true;
	KopeteContactPtrList p;
	p.append(this);
	KopeteMessage msg = KopeteMessage(this, p, i18n("Direct connection to %1 established").arg(mName), KopeteMessage::Internal, KopeteMessage::PlainText ) ;
	manager()->appendMessage(msg);
}

/** Called when the direct connection to contact @name has been terminated */
void OscarContact::slotDirectIMConnectionClosed(QString name)
{
	// Check if we're the one who is directly connected
	if ( tocNormalize(name) != tocNormalize(mName) )
		return;

	kdDebug(14150) << "[OscarContact] Setting direct connect state for " << mName << " to false." << endl;
	mDirectlyConnected = false;
}

/** Sends a file */
void OscarContact::sendFile(const KURL &sourceURL, const QString &/*altFileName*/,
	const long unsigned int /*fileSize*/)
{
	KURL filePath;

	//If the file location is null, then get it from a file open dialog
	if( !sourceURL.isValid() )
		filePath = KFileDialog::getOpenURL( QString::null ,"*.*", 0l  , i18n( "Kopete File Transfer" ));
	else
		filePath = sourceURL;

	if ( !filePath.isEmpty() )
	{
		KFileItem finfo(KFileItem::Unknown, KFileItem::Unknown, filePath);
		kdDebug(14150) << "[OscarContact] File size is " << (unsigned long)finfo.size() << endl;
		//Send the file
		mProtocol->engine->sendFileSendRequest( mName, finfo );
	}
}

/** Called when someone wants to send us a file */
void OscarContact::slotGotFileSendRequest(QString sn, QString message, QString filename, unsigned long filesize)
{
	// Check if we're the one who is directly connected
	if ( tocNormalize(sn) != tocNormalize(mName) )
		return;

	kdDebug(14150) << "[OscarContact] Got file x-fer request for " << mName << endl;
	KopeteTransferManager::transferManager()->askIncomingTransfer(this, filename, filesize, message);
}

/** Called when a pending transfer has been accepted */
void OscarContact::slotTransferAccepted(KopeteTransfer *tr, const QString &fileName)
{
	// Check if we're the one who is directly connected
	if ( tr->info().contact() != this )
		return;

	kdDebug(14150) << k_funcinfo << "Transfer of " << fileName << " accepted." << endl;
	OscarConnection *fs = mProtocol->engine->sendFileSendAccept(mName, fileName);

	//connect to transfer manager
	QObject::connect( fs, SIGNAL( percentComplete( unsigned int ) ),
		tr, SLOT(slotPercentCompleted( unsigned int )) );
}

/** Called when we deny a transfer */
void OscarContact::slotTransferDenied(const KopeteFileTransferInfo &tr)
{
	// Check if we're the one who is directly connected
	if ( tr.contact() != this )
		return;

	kdDebug(14150) << k_funcinfo << "Transfer denied." << endl;
	mProtocol->engine->sendFileSendDeny(mName);
}

/** Called when a file transfer begins */
void OscarContact::slotTransferBegun(OscarConnection *con, const QString& file, const unsigned long size, const QString &recipient)
{
	if ( tocNormalize(con->connectionName()) != tocNormalize(mName) )
		return;

	kdDebug(14150) << k_funcinfo << "adding transfer of " << file << endl;
	KopeteTransfer *tr = KopeteTransferManager::transferManager()->addTransfer( this, file, size, recipient, KopeteFileTransferInfo::Outgoing );

	//connect to transfer manager
	QObject::connect( con, SIGNAL( percentComplete( unsigned int ) ),
		tr, SLOT(slotPercentCompleted( unsigned int )) );
}

/*	if ( (status() != m_cachedOldStatus) || ( size != m_cachedSize ) )
	{
		QImage afScal = ((QPixmap(SmallIcon(statusIcon()))).convertToImage()).smoothScale( size, size );
		m_cachedScaledIcon = QPixmap(afScal);
		m_cachedOldStatus = status();
		m_cachedSize = size;
	}
	if ( m_idleState == Idle || mDirectlyConnected )
	{
		QImage tmp;
		QPixmap pm;
		tmp = m_cachedScaledIcon.convertToImage();
		if ( m_idleState == Idle ) // if this contact is idle, make its icon semi-transparent
			KIconEffect::semiTransparent(tmp);
		if ( mDirectlyConnected ) //if we are directly connected, change color of icon
			KIconEffect::colorize(tmp, red, 1.0F);
		pm.convertFromImage(tmp);
		return pm;
	}
	return m_cachedScaledIcon; */

// vim: set noet ts=4 sts=4 sw=4:

#include "oscarcontact.moc"

