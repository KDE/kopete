//
//
// C++ Implementation: cpp
//
// Description:
//
//
// Author: Will Stephenson, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <time.h>

#include <qapplication.h>
#include <qregexp.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "kopeteaway.h"
#include "kopetemessagemanager.h"
#include "aimprotocol.h"
#include "aimcontact.h"
#include "aimaccount.h"
#include "aimuserinfo.h"
#include "aim.h" //for tocNormalize

AIMContact::AIMContact(const QString name, const QString displayName, AIMAccount *account, KopeteMetaContact *parent)
	 : OscarContact(name, displayName, account, parent)
{
	mProtocol = static_cast<AIMProtocol *>(protocol());
	setOnlineStatus(mProtocol->statusOffline);

	mLastAutoResponseTime = 0;
	mUserProfile = "";
	infoDialog=0L;

	// Contact Changed
	QObject::connect(
		account->engine(), SIGNAL(gotBuddyChange(const UserInfo &)),
		this, SLOT(slotContactChanged(const UserInfo &)));
	// Received IM
	QObject::connect(
		account->engine(), SIGNAL(gotIM(QString,QString,bool)),
		this, SLOT(slotIMReceived(QString,QString,bool)));
	// Incoming minitype notification
	QObject::connect(
		account->engine(), SIGNAL(gotMiniTypeNotification(QString, int)),
		this, SLOT(slotGotMiniType(QString, int)));
	// received userprofile
	QObject::connect(
		account->engine(), SIGNAL(gotUserProfile(const UserInfo &, const QString &, const QString &)),
		this, SLOT(slotGotProfile(const UserInfo &, const QString &, const QString &)));

// 	kdDebug(14190) << k_funcinfo "name='" << name <<
// 		"', displayName='" << displayName << "' " << endl;
}

void AIMContact::setOwnProfile(const QString &profile)
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;
	if(this == account()->myself())
	{
		mUserProfile = profile;
		if(mAccount->isConnected())
			mAccount->engine()->sendLocationInfo(mUserProfile);
	}
}

void AIMContact::slotGotProfile(const UserInfo &user, const QString &profile, const QString &away)
{
	if(tocNormalize(user.sn) != tocNormalize(mName))
		return;
	kdDebug(14200) << k_funcinfo << "Called for contact '" << displayName() << "'" << endl;
	mUserProfile = profile;
	mAwayMessage = away;
	mUserInfo = user;
	emit updatedProfile();
}


AIMContact::~AIMContact()
{
}

bool AIMContact::isReachable()
{
	return isOnline();
}

KActionCollection *AIMContact::customContextMenuActions()
{
	actionCollection = new KActionCollection(this);

	KAction* actionWarn = new KAction(i18n("&Warn"), 0,
		this, SLOT(slotWarn()), actionCollection, "actionWarn");
	KAction* actionBlock = new KAction(i18n("&Block"), 0,
		this, SLOT(slotBlock()), actionCollection, "actionBlock");
	KAction* actionDirectConnect = new KAction(i18n("&Direct IM"), 0,
		this, SLOT(slotDirectConnect()), actionCollection, "actionDirectConnect");

	actionCollection->insert(actionWarn);
	actionCollection->insert(actionBlock);
	actionCollection->insert(actionDirectConnect);

	return actionCollection;
}
/*
KopeteMessageManager* AIMContact::manager(bool)
{
	// Check to see if we already have a message manager
	if (mMsgManager == 0L)
	{  // We don't have one, so create it
		// The true flag here is to tell OscarContact that
		// it can create the message mananger if it
		// doesn't exist, which is the case here.
		OscarContact::manager( true );
		// Connect the typing signal to the slot here
		QObject::connect( mMsgManager, SIGNAL(typingMsg(bool)),
						  this, SLOT(slotTyping(bool)));
	}

	// Return the message manager
	return mMsgManager;
}*/

void AIMContact::setStatus(const unsigned int newStatus)
{
	if(onlineStatus().internalStatus() == newStatus)
		return;

	switch(newStatus)
	{
		case OSCAR_OFFLINE:
			setOnlineStatus(mProtocol->statusOffline);
			break;
		case OSCAR_AWAY:
			setOnlineStatus(mProtocol->statusAway);
			break;
		case OSCAR_CONNECTING:
			setOnlineStatus(mProtocol->statusConnecting);
			break;
		default: // emergency choose, also OSCAR_ONLINE
			setOnlineStatus(mProtocol->statusOnline);
	}

//	kdDebug(14190) << k_funcinfo << "'" << displayName() << "' is now " <<
//		onlineStatus().description() << endl;
}

void AIMContact::slotTyping(bool typing)
{
//	kdDebug(14190) << k_funcinfo << "Typing: " << typing << endl;
	mAccount->engine()->sendMiniTypingNotify(tocNormalize(mName),
		typing ? OscarSocket::TypingBegun : OscarSocket::TypingFinished );
}

// Called when we get a minityping notification
void AIMContact::slotGotMiniType(QString screenName, int type)
{
	//TODO
	// Check to see if it's us
	if(tocNormalize(screenName) != contactName())
		return;

	kdDebug(14190) << k_funcinfo << "Got minitype notification for " << mName << endl;

	// If we already have a message manager
	if( mMsgManager )
	{
		// Switch on the type
		switch(type)
		{
		case 0: case 1:
			// 0 == Typing Finished
			// 1 == Text Typed
			// Both of these are types of "not typing"
			mMsgManager->receivedTypingMsg(this, false);
			break;
		case 2:
			// Typing started
			mMsgManager->receivedTypingMsg(this, true);
			break;
		default:
			break;
		}
	}
}

void AIMContact::slotContactChanged(const UserInfo &u)
{
	if (tocNormalize(u.sn) != contactName())
		return; //this is not this contact

//	kdDebug(14190) << k_funcinfo << "Called for '"
//		<< displayName() << "', userclass=" << u.userclass << endl;

	if(u.userclass & CLASS_AIM)
		kdDebug(14190) << k_funcinfo << "AIM user" << endl;
	if(u.userclass & CLASS_ICQ)
		kdDebug(14190) << k_funcinfo << "ICQ user??" << endl;
	if(u.userclass & CLASS_WIRELESS)
		kdDebug(14190) << k_funcinfo << "Wireless user" << endl;
	if(u.userclass & CLASS_COMMERCIAL)
		kdDebug(14190) << k_funcinfo << "AOL commercial account" << endl;
	if(u.userclass & CLASS_TRIAL)
		kdDebug(14190) << k_funcinfo << "AOL trial account" << endl;
	if(u.userclass & CLASS_ADMINISTRATOR)
		kdDebug(14190) << k_funcinfo << "AOL administrator account" << endl;
	if(u.userclass & CLASS_UNKNOWN400)
		kdDebug(14190) << k_funcinfo << "Active contact" << endl;

	if(u.userclass & CLASS_AWAY)
		setStatus(OSCAR_AWAY);
	else
		setStatus(OSCAR_ONLINE);

	mUserInfo = u;

	slotUpdateBuddy();
}

void AIMContact::slotOffgoingBuddy(QString sn)
{
	if(tocNormalize(sn) != contactName())
		return;

	kdDebug(14190) << k_funcinfo << "Called for '" << displayName() << "'" << endl;

	setOnlineStatus(mProtocol->statusOffline);
	slotUpdateBuddy();
}

// Called when an IM is received

void AIMContact::slotIMReceived( QString message, QString sender, bool /* isAuto */ )
{
	// Check if we're the one who sent the message
	if(tocNormalize(sender) != contactName())
		return;

	// Tell the message manager that the buddy is done typing
	manager()->receivedTypingMsg( this, false );

	// Build a KopeteMessage and set the body as Rich Text
	KopeteContactPtrList tmpList;
	tmpList.append(mAccount->myself());
	KopeteMessage msg = parseAIMHTML( message );
	manager()->appendMessage(msg);

	// send our away message in fire-and-forget-mode :)
	if(mAccount->isAway())
	{
		// Get the current time
		long currentTime = time(0L);

		// Compare to the last time we sent a message
		// We'll wait 2 minutes between responses
		if( (currentTime - mLastAutoResponseTime) > 120 )
		{
			kdDebug(14190) << k_funcinfo << " while we are away, " \
				"sending away-message to annoy buddy :)" << endl;
			// Send the autoresponse
			mAccount->engine()->sendIM(
				KopeteAway::getInstance()->message(),
				mName, true);
			// Build a pointerlist to insert this contact into
			KopeteContactPtrList toContact;
			toContact.append(this);
			// Display the autoresponse
			// Make it look different
			// FIXME: hardcoded color
			QString responseDisplay =
				"<font color='#666699'>Autoresponse: </font>" +
				KopeteAway::getInstance()->message();

			KopeteMessage message( mAccount->myself(), toContact,
				responseDisplay, KopeteMessage::Outbound, KopeteMessage::RichText);

			manager()->appendMessage(message);
			// Set the time we last sent an autoresponse
			// which is right now
			mLastAutoResponseTime = time(0L);
		}
	}
}

void AIMContact::slotSendMsg(KopeteMessage& message, KopeteMessageManager *)
{
	QString plainMessage = message.plainBody();

	if (plainMessage.isEmpty()) // no text, do nothing
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
	if (
			( mListContact->status() == static_cast<int>( OSCAR_OFFLINE ) ) ||
			( onlineStatus().status() == KopeteOnlineStatus::Offline )
		)
	{
		KMessageBox::sorry(qApp->mainWidget(),
			i18n("<qt>This user is not online at the moment for you to message him/her. "
				"AIM users must be online for you to be able to message them.</qt>"),
			i18n("User not Online"));
		return;
	}

	// FIXME: We don't do HTML in ICQ
	// we might be able to do that in AIM and we might also convert
	// HTML to RTF for ICQ type-2 messages  [mETz]
	// Will asks: Does this still apply in AIM?
	mAccount->engine()->sendIM(plainMessage, mName, false);

	// Show the message we just sent in the chat window
	manager()->appendMessage(message);
	manager()->messageSucceeded();
}

KopeteMessage AIMContact::parseAIMHTML ( QString m )
{
/*	============================================================================================
	Original AIM-Messages, just a few to get the idea of the weird format[tm]:

	From original AIM: --------------------------------------------------------------------
	<HTML><BODY BGCOLOR="#ffffff"><FONT FACE="Verdana" SIZE=4>some text message</FONT></BODY></HTML>
	From Trillian 0.7something: ---------------------------------------------
	<HTML><BODY BGCOLOR="#ffffff"><font face="Arial"><b>bin ich ueberhaupt ein standard?</b></BODY></HTML>
	<HTML><BODY BGCOLOR="#ffffff"><font face="Arial"><font color="#ffff00">ups</BODY></HTML>
	<HTML><BODY BGCOLOR="#ffffff"><font face="Arial"><font back="#00ff00">bggruen</BODY></HTML>
	<HTML><BODY BGCOLOR="#ffffff"><font face="Arial"><font back="#00ff00"><font color="#ffff00">both</BODY></HTML>
	<HTML><BODY BGCOLOR="#ffffff"><font face="Arial">LOL</BODY></HTML>
	From GAIM: ----------------------------------------------------------
	<FONT COLOR="#0002A6"><FONT SIZE="2">cool cool</FONT></FONT>
	============================================================================================ */

	kdDebug(14190) << k_funcinfo << "Original MSG: " << m << endl;

	QString result = m;
	result.replace( QRegExp(
		QString::fromLatin1("<[hH][tT][mM][lL].*>(.*)</[hH][tT][mM][lL]>") ),
		QString::fromLatin1("\\1") );
	result.replace( QRegExp(
		QString::fromLatin1("<[bB][oO][dD][yY].*>(.*)</[bB][oO][dD][yY]>") ),
		QString::fromLatin1("\\1") );
	result.replace( QRegExp(
		QString::fromLatin1("<[bB][rR]>") ),
		QString::fromLatin1("<br/>") );

	KopeteContactPtrList tmpList;
	tmpList.append(mAccount->myself());
	KopeteMessage msg( this, tmpList, result, KopeteMessage::Inbound, KopeteMessage::RichText);

	// We don't actually do anything in there yet, but we might eventually
	return msg;
}

void AIMContact::slotUserInfo()
{
	if (!infoDialog)
	{
		infoDialog = new AIMUserInfoDialog(this, static_cast<AIMAccount*>(account()),
			false, 0L, displayName()+"_userInfoDialog");
		if(!infoDialog)
			return;
		connect(infoDialog, SIGNAL(closing()), this, SLOT(slotCloseUserInfoDialog()));
		infoDialog->show();
	}
	else
	{
		infoDialog->raise();
	}
}

void AIMContact::slotCloseUserInfoDialog()
{
	infoDialog->delayedDestruct();
	infoDialog = 0L;
}

void AIMContact::slotWarn()
{
	QString message = i18n( "<qt>Would you like to warn %1 anonymously or with your name?<br>" \
		"(Warning a user on AIM will result in a \"Warning Level\"" \
		" increasing for the user you warn. Once this level has reached a" \
		" certain point, they will not be able to sign on. Please do not abuse" \
		" this function, it is meant for legitimate practices.)</qt>" ).arg(mName);

	int result = KMessageBox::questionYesNoCancel(
		qApp->mainWidget(),
		message,
		i18n("Warn User %1?").arg(mName),
		i18n("Warn Anonymously"),
		i18n("Warn"));

	if (result == KMessageBox::Yes)
		mAccount->engine()->sendWarning(mName, true);
	else if (result == KMessageBox::No)
		mAccount->engine()->sendWarning(mName, false);
}
#include "aimcontact.moc"
