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
#include "kopeteuiglobal.h"
#include "aimbuddy.h"
#include "aimprotocol.h"
#include "aimcontact.h"
#include "aimaccount.h"
#include "aimuserinfo.h"
#include "aim.h" //for tocNormalize

AIMContact::AIMContact(const QString name, const QString displayName, AIMAccount *acc, KopeteMetaContact *parent)
	 : OscarContact(name, displayName, acc, parent)
{
	mProtocol=static_cast<AIMProtocol *>(protocol());
	setOnlineStatus(mProtocol->statusOffline);

	mLastAutoResponseTime=0;
	mUserProfile="";
	infoDialog=0L;

	// Contact changed his online status
	QObject::connect(
		acc->engine(), SIGNAL(gotContactChange(const UserInfo &)),
		this, SLOT(slotContactChanged(const UserInfo &)));

	// Incoming minitype notification
	QObject::connect(
		acc->engine(), SIGNAL(gotMiniTypeNotification(const QString &, int)),
		this, SLOT(slotGotMiniType(const QString &, int)));

	// received userprofile
	QObject::connect(
		acc->engine(), SIGNAL(gotUserProfile(const UserInfo &, const QString &, const QString &)),
		this, SLOT(slotGotProfile(const UserInfo &, const QString &, const QString &)));

	/*kdDebug(14190) << k_funcinfo <<
		"contactName()='" << contactName() <<
		"', displayName()='" << displayName << "'" << endl;*/
	actionRequestAuth = 0L;
}

AIMContact::~AIMContact()
{
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
	if(tocNormalize(user.sn) != contactName())
		return;
	kdDebug(14200) << k_funcinfo << "Called for contact '" << displayName() << "'" << endl;
	mUserProfile = profile;
	mAwayMessage = away;
	emit updatedProfile();
}

bool AIMContact::isReachable()
{
	return isOnline();
}

QPtrList<KAction> *AIMContact::customContextMenuActions()
{
	QPtrList<KAction> *actionCollection = new QPtrList<KAction>();

	if( !actionRequestAuth )
	{
		actionRequestAuth = new KAction(i18n("&Request Authorization"), "mail_reply", 0,
			this, SLOT(slotRequestAuth()), this, "actionRequestAuth");
		actionSendAuth = new KAction(i18n("&Send Authorization"), "mail_forward", 0,
			this, SLOT(slotSendAuth()), this, "actionSendAuth");
		actionWarn = new KAction(i18n("&Warn"), 0,
			this, SLOT(slotWarn()), this, "actionWarn");
		actionBlock = new KAction(i18n("&Block"), 0,
			this, SLOT(slotBlock()), this, "actionBlock");
		/*KAction* actionDirectConnect = new KAction(i18n("&Direct IM"), 0,
			this, SLOT(slotDirectConnect()), this, "actionDirectConnect");*/
	}

	actionRequestAuth->setEnabled(isOnline());
	actionSendAuth->setEnabled(isOnline());
	actionWarn->setEnabled(isOnline());
	actionBlock->setEnabled(mAccount->isConnected()); // works if contact is offline

	actionCollection->append(actionRequestAuth);
	actionCollection->append(actionSendAuth);
	actionCollection->append(actionWarn);
	actionCollection->append(actionBlock);
	//actionCollection->insert(actionDirectConnect);

	return actionCollection;
}

KopeteMessageManager* AIMContact::manager(bool)
{
	//kdDebug(14190) << k_funcinfo << "called" << endl;
	// Check to see if we already have a message manager
	if (!mMsgManager)
	{
		// The true flag here is to tell OscarContact that
		// it can create the message mananger if it
		// doesn't exist, which is the case here.
		OscarContact::manager(true);

		// Connect the typing signal to the slot here
		QObject::connect(mMsgManager, SIGNAL(typingMsg(bool)),
			this, SLOT(slotTyping(bool)));
	}

	// Return the message manager
	return mMsgManager;
}

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
	mAccount->engine()->sendMiniTypingNotify(contactName(),
		typing ? OscarSocket::TypingBegun : OscarSocket::TypingFinished );
}

// Called when we get a minityping notification
void AIMContact::slotGotMiniType(const QString &screenName, int type)
{
	// Check to see if it's us
	if(tocNormalize(screenName) != contactName())
		return;

//	kdDebug(14190) << k_funcinfo << "Got minitype notification for " << contactName() << endl;

	// Only if we already have a message manager
	if(mMsgManager == 0L)
		return;

	switch(type)
	{
		case 0:
		case 1:
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

void AIMContact::slotContactChanged(const UserInfo &u)
{
	if (tocNormalize(u.sn) != contactName())
		return; //this is not this contact

	/*kdDebug(14190) << k_funcinfo << "Called for '"
		<< displayName() << "', contactName()=" << contactName() << endl;*/

	/*
	QString uclass = "";
	if(u.userclass & CLASS_AWAY)
		uclass += " AWAY ";
	else
		uclass += " ONLINE ";
	if(u.userclass & CLASS_AIM)
		uclass += " AIM User ";
	if(u.userclass & CLASS_ICQ)
		uclass += " ICQ user ";
	if(u.userclass & CLASS_WIRELESS)
		uclass += " Wireless user ";
	if(u.userclass & CLASS_COMMERCIAL)
		uclass += " AOL commercial account ";
	if(u.userclass & CLASS_TRIAL)
		uclass += " AOL trial account ";
	if(u.userclass & CLASS_ADMINISTRATOR)
		uclass += " AOL administrator account ";
	if(u.userclass & CLASS_UNKNOWN400)
		uclass += " Active contact ";

	kdDebug(14190) << k_funcinfo << "decoded userclass=[" << uclass << "]" << endl;
	*/

	if(u.userclass & CLASS_AWAY)
	{
		if((this != account()->myself()) &&
		(account()->myself()->onlineStatus().status() != KopeteOnlineStatus::Connecting))
		{
			// TODO: Add queues for away message requests
			// request away message
			mAccount->engine()->sendUserLocationInfoRequest(contactName(), AIM_LOCINFO_AWAYMESSAGE);
		}
		setStatus(OSCAR_AWAY);
	}
	else
		setStatus(OSCAR_ONLINE);

	slotUpdateBuddy();
}

void AIMContact::slotOffgoingBuddy(QString sn)
{
	if(tocNormalize(sn) != contactName())
		return;

	/*kdDebug(14190) << k_funcinfo << "Called for '"
		<< displayName() << "', contactName()=" << contactName() << endl;*/

	setStatus(OSCAR_OFFLINE);
	slotUpdateBuddy();
}

#if 0
void AIMContact::gotIM(OscarSocket::OscarMessageType /*type*/, const QString &message)
{
	// Tell the message manager that the buddy is done typing
	manager()->receivedTypingMsg(this, false);

	// Build a KopeteMessage and set the body as Rich Text
	KopeteMessage msg = parseAIMHTML(message);
	manager()->appendMessage(msg);

	// send our away message in fire-and-forget-mode :)
	if(mAccount->isAway())
	{
		// Compare current time to last time we sent a message
		// We'll wait 2 minutes between responses
		if((time(0L) - mLastAutoResponseTime) > 120)
		{
			kdDebug(14190) << k_funcinfo << " while we are away, " \
				"sending away-message to annoy buddy :)" << endl;
			// Send the autoresponse
			mAccount->engine()->sendIM(KopeteAway::getInstance()->message(), this, true);
			// Build a pointerlist to insert this contact into
			KopeteContactPtrList toContact;
			toContact.append(this);
			// Display the autoresponse
			// Make it look different
			// FIXME: hardcoded color
			QString responseDisplay =
				"<font color='#666699'>Autoresponse: </font>" +
				KopeteAway::getInstance()->message();

			KopeteMessage message(mAccount->myself(), toContact,
				responseDisplay, KopeteMessage::Outbound, KopeteMessage::RichText);

			manager()->appendMessage(message);
			// Set the time we last sent an autoresponse
			// which is right now
			mLastAutoResponseTime = time(0L);
		}
	}
}
#endif

void AIMContact::slotSendMsg(KopeteMessage& message, KopeteMessageManager *)
{
	if (message.plainBody().isEmpty()) // no text, do nothing
		return;

	// ===================================================================================

	QString finalMessage = "<HTML>";
	if(message.bg().isValid())
		finalMessage += "<BODY BGCOLOR=\"" + message.bg().name() + "\">";
	else
		finalMessage += "<BODY>";
	if(message.fg().isValid())
		finalMessage += "<FONT COLOR=\"" + message.fg().name() + "\">";
	if(!message.font().family().isEmpty())
		finalMessage += "<FONT FACE=\"" + message.font().family() + "\">";

	finalMessage += message.escapedBody().replace("<br />" , "<br>");

	if(!message.font().family().isEmpty())
		finalMessage += "</FONT>";
	if(message.fg().isValid())
		finalMessage += "</FONT>";
	finalMessage += "</BODY></HTML>";

	// ===================================================================================

	// Check to see if we're even online
	if(!mAccount->isConnected())
	{
		KMessageBox::sorry(Kopete::UI::Global::mainWidget(),
			i18n("<qt>You must be logged on to AIM before you can send a message to a user.</qt>"),
			i18n("Not Signed On"));
		return;
	}

	// Check to see if the person we're sending the message to is online
	if((mListContact->status() == static_cast<int>(OSCAR_OFFLINE)) ||
		(onlineStatus().status() == KopeteOnlineStatus::Offline))
	{
		KMessageBox::sorry(Kopete::UI::Global::mainWidget(),
			i18n("<qt>This user is not online at the moment for you to message them. "
				"AIM users must be online for you to be able to message them.</qt>"),
			i18n("User Not Online"));
		return;
	}

	// FIXME: We don't do HTML in ICQ
	// we might be able to do that in AIM and we might also convert
	// HTML to RTF for ICQ type-2 messages  [mETz]
	// Will asks: Does this still apply in AIM?
	mAccount->engine()->sendIM(finalMessage, this, false);

	// Show the message we just sent in the chat window
	manager()->appendMessage(message);
	manager()->messageSucceeded();
}

#if 0
KopeteMessage AIMContact::parseAIMHTML(const QString &m)
{
/* ========================================================================================
Original AIM-Messages, just a few to get the idea of the weird format[tm]:

From original AIM: ------------------------------------------------------------------------
<HTML><BODY BGCOLOR="#ffffff"><FONT FACE="Verdana" SIZE=4>some text message</FONT></BODY></HTML>

From Trillian 0.7something: ---------------------------------------------------------------
<HTML><BODY BGCOLOR="#ffffff"><font face="Arial"><font color="#ffff00">ups</BODY></HTML>
<HTML><BODY BGCOLOR="#ffffff"><font face="Arial"><font back="#00ff00">bggruen</BODY></HTML>
<HTML><BODY BGCOLOR="#ffffff"><font face="Arial"><font back="#00ff00"><font color="#ffff00">both</BODY></HTML>

From GAIM: --------------------------------------------------------------------------------
<FONT COLOR="#0002A6"><FONT SIZE="2">cool cool</FONT></FONT>

Original AIM 5.2 message: -----------------------------------------------------------------
<HTML><BODY BGCOLOR="#ffffff">
<FONT LANG="0" SIZE=2>t</FONT>
<FONT BACK="#ffffff" SIZE=3>h</FONT>
<FONT SIZE=4>i</FONT>
<FONT SIZE=5>s</FONT>
<FONT SIZE=6> i</FONT>
<FONT SIZE=7>s <B><I><U>a </B></I></U> </FONT>
<FONT BACK="#008000" SIZE=7>test</FONT>
<FONT BACK="#ffffff" SIZE=7> <I></FONT>
<FONT SIZE=3>of <B>h</I>t<U>m</U>l</B></FONT>
<FONT COLOR="#008000"> formatting</FONT>
</BODY></HTML>

AIM 5.2 with fg and bg set for some text: -------------------------------------------------
<HTML><BODY BGCOLOR="#ffffff">
<FONT COLOR="#00ff00" BACK="#800080" LANG="0">test</FONT>
</BODY></HTML>
=========================================================================================== */

//	kdDebug(14190) << k_funcinfo << "Original MSG: '" << m << "'" << endl;

	QString result = m;
	result.replace( QRegExp(
		QString::fromLatin1("<[hH][tT][mM][lL].*>(.*)</[hH][tT][mM][lL]>") ),
		QString::fromLatin1("\\1") );
	result.replace( QRegExp(
		QString::fromLatin1("<[bB][oO][dD][yY].*>(.*)</[bB][oO][dD][yY]>") ),
		QString::fromLatin1("\\1") );
	result.replace( QRegExp(
		QString::fromLatin1("<[bB][rR]>") ),
		QString::fromLatin1("<br />") );
	result.replace( QRegExp(
		QString::fromLatin1("<[fF][oO][nN][tT].*[bB][aA][cC][kK]=(.*).*>") ),
		QString::fromLatin1("<span style=\"background-color:\\1 ;\"") );
	result.replace( QRegExp(
		QString::fromLatin1("</[fF][oO][nN][tT]>") ),
		QString::fromLatin1("</span>") );

//	kdDebug(14190) << k_funcinfo << "Final MSG: '" << result << "'" << endl;

	KopeteContactPtrList tmpList;
	tmpList.append(mAccount->myself());
	KopeteMessage msg(this, tmpList, result, KopeteMessage::Inbound, KopeteMessage::RichText);

	return msg;
}
#endif

void AIMContact::slotUserInfo()
{
	if (!infoDialog)
	{
		infoDialog = new AIMUserInfoDialog(this, static_cast<AIMAccount*>(account()),
			false, 0L, ( displayName() + "_userInfoDialog" ).latin1() );
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
		" this function, it is meant for legitimate practices.)</qt>" ).arg(displayName());

	int result = KMessageBox::questionYesNoCancel(
		Kopete::UI::Global::mainWidget(),
		message,
		i18n("Warn User %1?").arg(displayName()),
		i18n("Warn Anonymously"),
		i18n("Warn"));

	if (result == KMessageBox::Yes)
		mAccount->engine()->sendWarning(contactName(), true);
	else if (result == KMessageBox::No)
		mAccount->engine()->sendWarning(contactName(), false);
}
#include "aimcontact.moc"
