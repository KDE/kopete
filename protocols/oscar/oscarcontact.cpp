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
#include "kopeteaccount.h"

#include "aimbuddylist.h"
#include "oscarprotocol.h"
#include "oscarsocket.h"
#include "oscaruserinfo.h"
#include "oscaraccount.h"

OscarContact::OscarContact(const QString name, const QString displayName,
			   KopeteAccount *account, KopeteMetaContact *parent)
    : KopeteContact(account, name, parent)
{
    kdDebug(14150) << "[OscarContact] OscarContact(), name=" << name << endl;

    // Save the OscarAccount object using static_cast
    if (!account) {
	kdDebug(14150) << "[OscarContact] Account pointer was null!" << endl;
    }

    kdDebug(14150) << "[OscarContact] Casting account to OscarAccount" <<  endl;

    mAccount = static_cast<OscarAccount*>(account);

    // We store normalized names (lowercase no spaces)
    mName = tocNormalize(name);
    mMsgManager = 0L;
    kdDebug(14150) << "[OscarContact] Attempting to get the internal buddy list for this account" << endl;
    mListContact = mAccount->internalBuddyList()->findBuddy(mName);
    mIdle = 0;
    mLastAutoResponseTime = 0;
    setFileCapable(true);

    kdDebug(14150) << "[OscarContact] Setting initial status" << endl;
    setOnlineStatus(OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::OFFLINE) );
    kdDebug(14150) << "[OscarContact] Status set to Offline" << endl;

    // Sets our display name (from KopeteContact)
    if (displayName != QString::null)
    {
	kdDebug(14150) << "[OscarContact] Setting display name to: " << displayName << endl;
	setDisplayName(displayName);
    }
    else
    {
	kdDebug(14150) << "[OscarContact] Setting display name to: " << name << endl;
	setDisplayName(name);
    }


    if (!mListContact)
    {
	kdDebug(14150) << "[OscarContact] Internal buddy list did not contain this contact, creating " << endl;
	mListContact = new AIMBuddy(mAccount->randomNewBuddyNum(), 0, mName);
	mAccount->internalBuddyList()->addBuddy(mListContact);
	// Do a double check
	if (!mListContact)
	    kdDebug(14150) << "[OSCAR] ERROR, mListContact is *STILL* NULL! Prepare to crashz0r!" << endl;
    }

    initSignals();

    kdDebug(14150) << "[OscarContact] Initializing actions for  " << mName << endl;
    initActions();
    kdDebug(14150) << "[OscarContact] Finished initializing actions for  " << mName << endl;

//     if ( !(mListContact->alias().isEmpty()) )
//     {
// 	kdDebug(14150) << "[OscarContact] Setting display name to: " << mListContact->alias() << endl;
// 	setDisplayName( mListContact->alias() );
//     }
//     else
//     {
// 	kdDebug(14150) << "[OscarContact] Setting display name to: " << mListContact->screenname() << endl;
// 	setDisplayName( mListContact->screenname() );
//     }

    // Can't update buddy when buddy doesn't exist yet....
    //kdDebug(14150) << "[OscarContact] Updating buddy" << endl;
    //slotUpdateBuddy();

    kdDebug(14150) << "[OscarContact]  " << mName << endl;
    theContacts.append( this );
}


void OscarContact::initSignals()
{
    kdDebug(14150) << "[OscarContact] Connecting slots to signals for contact: " << mName << endl;

    kdDebug(14150) << "[OscarContact] Engine::gotBuddyChange" << endl;
    // Buddy Changed
    QObject::connect(mAccount->getEngine(),
		     SIGNAL(gotBuddyChange(UserInfo)),
		     this,
		     SLOT(slotBuddyChanged(UserInfo)));

    kdDebug(14150) << "[OscarContact] Engine::gotOffgoingBuddy" << endl;
    // Buddy offline
    QObject::connect(mAccount->getEngine(),
		     SIGNAL(gotOffgoingBuddy(QString)),
		     this,
		     SLOT(slotOffgoingBuddy(QString)));

    kdDebug(14150) << "[OscarContact] Engine::gotIM" << endl;
    // Got IM
    QObject::connect(mAccount->getEngine(),
		     SIGNAL(gotIM(QString,QString,bool)),
		     this,
		     SLOT(slotIMReceived(QString,QString,bool)));

    kdDebug(14150) << "[OscarContact] Engine::statusChange" << endl;
    // User's status changed (I don't understand this)
    QObject::connect( mAccount->getEngine(),
		      SIGNAL(statusChanged(const KopeteOnlineStatus &)),
		      SLOT(slotMainStatusChanged(const KopeteOnlineStatus &)));

    kdDebug(14150) << "[OscarContact] Engine::gotMiniTypeNotification" << endl;
    // Incoming minitype notification
    QObject::connect(mAccount->getEngine(),
		     SIGNAL(gotMiniTypeNotification(QString, int)),
		     this,
		     SLOT(slotGotMiniType(QString, int)));

    kdDebug(14150) << "[OscarContact] Engine::connectionReady" << endl;
    // New direct connection
    QObject::connect(mAccount->getEngine(),
		     SIGNAL(connectionReady(QString)),
		     this,
		     SLOT(slotDirectIMReady(QString)));

    kdDebug(14150) << "[OscarContact] Engine::directIMConnectionClosed" << endl;
    // Direct connection closed
    QObject::connect(mAccount->getEngine(),
		     SIGNAL(directIMConnectionClosed(QString)),
		     this,
		     SLOT(slotDirectIMConnectionClosed(QString)));

    kdDebug(14150) << "[OscarContact] Engine::goFileSendRequest" << endl;
    // File transfer request
    QObject::connect(mAccount->getEngine(),
		     SIGNAL(gotFileSendRequest(QString,QString,QString,unsigned long)),
		     this,
		     SLOT(slotGotFileSendRequest(QString,QString,QString,unsigned long)));

    kdDebug(14150) << "[OscarContact] Engine::transferBegun" << endl;
    // File transfer started
    QObject::connect(mAccount->getEngine(),
		     SIGNAL(transferBegun(OscarConnection *,
					  const QString &,
					  const unsigned long,
					  const QString &)),
		     this,
		     SLOT(slotTransferBegun(OscarConnection *,
					    const QString &,
					    const unsigned long,
					    const QString &)));

    kdDebug(14150) << "[OscarContact] TransferManager::accepted" << endl;
    // File transfer manager stuff
    QObject::connect( KopeteTransferManager::transferManager(),
		      SIGNAL(accepted(KopeteTransfer *,
				      const QString &)),
		      this,
		      SLOT(slotTransferAccepted(KopeteTransfer *,
						const QString &)) );

    kdDebug(14150) << "[OscarContact] TransferManager::refused" << endl;
    // When the file transfer is refused
    QObject::connect( KopeteTransferManager::transferManager(),
		      SIGNAL(refused(const KopeteFileTransferInfo &)),
		      this,
		      SLOT(slotTransferDenied(const KopeteFileTransferInfo &)));

    kdDebug(14150) << "[OscarContact] this::contactDestroyed" << endl;
    // When the contact is being removed (whether from a group or not)
    QObject::connect( this,
		      SIGNAL( contactDestroyed( KopeteContact * )),
		      SLOT( slotContactDestroyed( KopeteContact * )));

    kdDebug(14150) << "[OscarContact] KopeteContactList::groupRemoved" << endl;
    // When a group in the contact list is being removed, we're notified
    QObject::connect(KopeteContactList::contactList(),
		     SIGNAL(groupRemoved( KopeteGroup * )),
		     this,
		     SLOT(slotGroupRemoved( KopeteGroup * )));

    kdDebug(14150) << "[OscarContact] Finished initializing signal connections" << endl;
}


// void OscarContact::serialize(QMap< QString, QString > &serializedData,
// 			     QMap< QString, QString > &addressBookData)
// {
//     // I believe, by default, our contact name and alias
//     // are already stored for us, so we don't really want to do anything
// }

OscarContact::~OscarContact()
{
}

KopeteMessageManager* OscarContact::manager( bool )
{
    if ( mMsgManager )
    {
	return mMsgManager;
    }
    else
    {
	mMsgManager =
	    KopeteMessageManagerFactory::factory()->create(
		mAccount->myself(), theContacts, OscarProtocol::protocol());
	// This is for when the user types a message and presses send
	QObject::connect(
	    mMsgManager,
	    SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager *)),
	    this,
	    SLOT(slotSendMsg(KopeteMessage&, KopeteMessageManager *)));

	// For when the message manager is destroyed
	QObject::connect(
	    mMsgManager,
	    SIGNAL(destroyed()),
	    this,
	    SLOT(slotMessageManagerDestroyed()));

	// For when the message manager tells us that the user is typing
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

void OscarContact::slotMainStatusChanged( const KopeteOnlineStatus &newStatus )
{
    if( newStatus == OscarProtocol::protocol()->getOnlineStatus( OscarProtocol::OFFLINE ))
    {
	setOnlineStatus(OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::OFFLINE) );

	// Try to do this, otherwise no big deal
	mListContact->setStatus(
	   OscarProtocol::protocol()->getOnlineStatus( OscarProtocol::OFFLINE ) );
    }
}

void OscarContact::slotUpdateBuddy()
{
    // status did not change, do nothing
    if( onlineStatus() == mListContact->status() &&
	mIdle == mListContact->idleTime() )
    {
	return;
    }

    // if we have become idle
    if ( mAccount->isConnected() )
    {
	if ( mListContact->idleTime() > 0 )
	{
	    kdDebug(14150) << "[OscarContact] setting " << mName << " idle! Idletime: " << mListContact->idleTime() << endl;
	    setIdleState(Idle);
	}
	// we have become un-idle
	else
	{
	    kdDebug(14150) << "[OscarContact] setting " << mName << " active!" << endl;
	    setIdleState(Active);
	}
	mIdle = mListContact->idleTime();
    }

    setOnlineStatus( mListContact->status() );
    kdDebug( 14150 ) << k_funcinfo << "Contact " << mName << " is now " << onlineStatus().internalStatus() << endl;

    if ( mAccount->isConnected() ) // oscar-plugin is online
    {
	if ( mName != mListContact->screenname() ) // contact changed his nickname
	{
	    if ( !mListContact->alias().isEmpty() )
		setDisplayName(mListContact->alias());
	    else
		setDisplayName(mListContact->screenname());
	}
    }
    else // oscar-account is offline so all users are offline too
    {
	mListContact->setStatus(
	   OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::OFFLINE) );
	setOnlineStatus(
	   OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::OFFLINE) );

	return;
    }
}

/** Initialzes the actions */
void OscarContact::initActions()
{
    actionCollection = 0L;

    kdDebug(14150) << "[OscarContact] Initializing Warn action" << endl;
    actionWarn = new KAction(i18n("&Warn"), 0, this, SLOT(slotWarn()), this, "actionWarn");
    kdDebug(14150) << "[OscarContact] Initializing Block action" << endl;
    actionBlock = new KAction(i18n("&Block"), 0, this, SLOT(slotBlock()), this, "actionBlock");
    kdDebug(14150) << "[OscarContact] Initializing DirectConnect action" << endl;
    actionDirectConnect = new KAction(i18n("&Direct IM"), 0, this, SLOT(slotDirectConnect()), this, "actionDirectConnect");
}

void OscarContact::slotBuddyChanged(UserInfo u)
{
    if (tocNormalize(u.sn) == tocNormalize(mName))
	//if we are the contact that is changing status
    {
	kdDebug(14150) << "[OscarContact] Setting status for " << u.sn << endl;
	if ( u.userclass & USERCLASS_AWAY )
	{
	    mListContact->setStatus(
		OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::AWAY));
	}
	else
	{
	    mListContact->setStatus(
		OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ONLINE));
	}

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

    mAccount->getEngine()->sendMiniTypingNotify( tocNormalize( mName ),
					     typing ? OscarSocket::TypingBegun : OscarSocket::TypingFinished );
}

void OscarContact::slotOffgoingBuddy(QString sn)
{
    if( tocNormalize( sn ) == tocNormalize( mName ) )
    {
	//if we are the contact that is offgoing
	mListContact->setStatus(
	   OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::OFFLINE));

	slotUpdateBuddy();
    }
}

/** Called when user info is requested */
void OscarContact::slotUserInfo(void)
{
    if (!mAccount->isConnected())
	KMessageBox::sorry(qApp->mainWidget(),
			   i18n("<qt>Sorry, you must be connected to the AIM server to retrieve user information, but you will be allowed to continue if you	would like to change the user's nickname.</qt>"),
			   i18n("You Must be Connected") );
    else
    {
	if( mListContact->status() ==OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::OFFLINE))
	{
	    KMessageBox::sorry( qApp->mainWidget(),
				i18n( "<qt>Sorry, this user isn't online for you to view his/her information, "
				      "but you will be allowed to only change his/her nickname. "
				      "Please wait until this user becomes available and try again</qt>" ),
				i18n( "User not Online" ) );
	}
    }

    OscarUserInfo *Oscaruserinfo =
	new OscarUserInfo(mName, mListContact->alias(), mAccount, *mListContact);

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
    tmpList.append(mAccount->myself());
    KopeteMessage msg = parseAIMHTML( message );
    manager()->appendMessage(msg);

    // send our away message in fire-and-forget-mode :)
    if ( mAccount->isAway() )
    {
	// Get the current time
	long currentTime = time(0L);

	// Compare to the last time we sent a message
	// We'll wait 2 minutes between responses
	if( (currentTime - mLastAutoResponseTime) > 120 )
	{
	    kdDebug(14150) << "[OscarContact] slotIMReceived() while we are away, sending away-message to annoy buddy :)" << endl;
	    // Send the autoresponse
	    mAccount->getEngine()->sendIM(
		KopeteAway::getInstance()->message(),
		mName, true);
	    // Build a pointerlist to insert this contact into
	    KopeteContactPtrList toContact;
	    toContact.append(this);
	    // Display the autoresponse
	    // Make it look different
	    QString responseDisplay = KopeteAway::getInstance()->message();
	    responseDisplay.prepend("<font color='#666699'>Autoresponse: </font>");

	    KopeteMessage message( mAccount->myself(), toContact,
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
    if (!mAccount->isConnected())
    {
	KMessageBox::sorry(qApp->mainWidget(),
			   i18n("<qt>You must be logged on to AIM before you can send a message to a user.</qt>"),
			   i18n("Not Signed On"));
	return;
    }

    // Check to see if the person we're sending the message to is online
    if( mListContact->status() ==
	OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::OFFLINE) ||
	onlineStatus() == OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::OFFLINE))
    {
	KMessageBox::sorry( qApp->mainWidget(),
			    i18n( "<qt>This user is not online at the moment for you to message him/her. "
				  "AIM users must be online for you to be able to message them.</qt>" ),
			    i18n( "User not Online" ) );
	return;
    }

    mAccount->getEngine()->sendIM(message.escapedBody(), mName, false);

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
bool OscarContact::isReachable()
{
    // With AIM  you have to be online to be reachable
    return isOnline();
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
    AIMGroup *group = mAccount->internalBuddyList()->findGroup(mListContact->groupID());
    if (!group) return;
    mAccount->internalBuddyList()->removeBuddy(mListContact);
    mAccount->getEngine()->sendDelBuddy(mListContact->screenname(),group->name());
    deleteLater();
}

void OscarContact::slotContactDestroyed( KopeteContact * /* contact */ )
{
    slotDeleteContact();
}

void OscarContact::slotGroupRemoved( KopeteGroup *group )
{
    kdDebug(14150) << "[OscarContact] slotGroupRemoved() being called" << endl;
    QString groupName=group->displayName();
    AIMGroup *aGroup = mAccount->internalBuddyList()->findGroup(mListContact->groupID());
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
	mAccount->getEngine()->sendWarning(mName, true);
    else if (result == KMessageBox::No)
	mAccount->getEngine()->sendWarning(mName, false);
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
    tmpList.append(mAccount->myself());
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
	mAccount->getEngine()->sendBlock(mName);
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
	mAccount->getEngine()->sendDirectIMRequest(mName);
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
    {
	filePath = KFileDialog::getOpenURL( QString::null ,"*.*", 0l,
					    i18n( "Kopete File Transfer" ));
    }
    else
    {
	filePath = sourceURL;
    }

    if ( !filePath.isEmpty() )
    {
	KFileItem finfo(KFileItem::Unknown, KFileItem::Unknown, filePath);
	kdDebug(14150) << "[OscarContact] File size is "
		       << (unsigned long)finfo.size() << endl;

	//Send the file
	mAccount->getEngine()->sendFileSendRequest( mName, finfo );
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
    OscarConnection *fs = mAccount->getEngine()->sendFileSendAccept(mName, fileName);

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
    mAccount->getEngine()->sendFileSendDeny(mName);
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

