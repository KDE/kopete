/*
  aimcontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Will Stephenson
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

#include <time.h>

#include <qapplication.h>
#include <qregexp.h>

#include <kactionclasses.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "kopeteaway.h"
#include "kopetemessagemanager.h"
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"
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

	mUserProfile="";
	infoDialog=0L;

	// Contact changed his online status
	QObject::connect(
		acc->engine(), SIGNAL(gotContactChange(const UserInfo &)),
		this, SLOT(slotContactChanged(const UserInfo &)));

	// Incoming minitype notification
	QObject::connect(
		acc->engine(), SIGNAL(gotMiniTypeNotification(const QString &, OscarConnection::TypingNotify)),
		this, SLOT(slotGotMiniType(const QString &, OscarConnection::TypingNotify)));

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
	setAwayMessage(away);
	if ( metaContact()->isTemporary() && onlineStatus().internalStatus() == OSCAR_OFFLINE && user.onlinesince.isValid() )
	{
		kdDebug(14150) << k_funcinfo << "Attempting to set status to online for temp contact" << endl;
		setStatus(OSCAR_ONLINE);
	}

	emit updatedProfile();
}

bool AIMContact::isReachable()
{
	return true;
	//return isOnline();
}

QPtrList<KAction> *AIMContact::customContextMenuActions()
{
	QPtrList<KAction> *actionCollection = new QPtrList<KAction>();

	if(!actionRequestAuth)
	{
		actionRequestAuth = new KAction(i18n("&Request Authorization"), "mail_reply", 0,
			this, SLOT(slotRequestAuth()), this, "actionRequestAuth");
		actionSendAuth = new KAction(i18n("&Send Authorization"), "mail_forward", 0,
			this, SLOT(slotSendAuth()), this, "actionSendAuth");
		actionWarn = new KAction(i18n("&Warn"), 0,
			this, SLOT(slotWarn()), this, "actionWarn");
		actionInvisibleTo = new KToggleAction(i18n("&Block"), "", 0,
			this, SLOT(slotInvisibleTo()), this, "actionInvisibleTo");
		/*KAction* actionDirectConnect = new KAction(i18n("&Direct IM"), 0,
			this, SLOT(slotDirectConnect()), this, "actionDirectConnect");*/
	}

	actionRequestAuth->setEnabled(isOnline());
	actionSendAuth->setEnabled(isOnline());
	actionWarn->setEnabled(isOnline());

	actionCollection->append(actionRequestAuth);
	actionCollection->append(actionSendAuth);
	actionCollection->append(actionWarn);
	actionCollection->append(actionInvisibleTo);
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
void AIMContact::slotGotMiniType(const QString &screenName, OscarConnection::TypingNotify type)
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
		case (OscarConnection::TypingFinished):
		case (OscarConnection::TextTyped): // Both of these are types of "not typing"
			mMsgManager->receivedTypingMsg(this, false);
			break;
		case (OscarConnection::TypingBegun): // Typing started
			mMsgManager->receivedTypingMsg(this, true);
			break;
		default:
			break;
	}
}

void AIMContact::slotContactChanged(const UserInfo &u)
{
	if (tocNormalize(u.sn) != contactName())
		return;

	// update mInfo and general stuff from OscarContact
	slotParseUserInfo(u);

	QStringList capList;
	if (hasCap(CAP_KOPETE))
		capList << i18n("Kopete %1").arg(mInfo.clientVersion);
	if (hasCap(CAP_MICQ))
		capList << i18n("MICQ");
		//capList << i18n("MICQ %1").arg(mInfo.clientVersion);
	if (hasCap(CAP_SIMNEW))
		capList << i18n("SIM %1").arg(mInfo.clientVersion);
	if (hasCap(CAP_MACICQ))
		capList << i18n("MacICQ");
	if (hasCap(CAP_BUDDYICON))
		capList << i18n("Buddyicons");
	if (hasCap(CAP_TRILLIANCRYPT))
		capList << i18n("Trillian Encryption");
	if (hasCap(CAP_UTF8))
		capList << i18n("UTF-8");
	if (hasCap(CAP_IS_WEB))
		capList << i18n("Lite/Web-Client");
	if (hasCap(CAP_RTFMSGS))
		capList << i18n("RTF-Messages");
	if (hasCap(CAP_CHAT))
		capList << i18n("Groupchat");
	if (hasCap(CAP_VOICE))
		capList << i18n("Voicechat");
	if (hasCap(CAP_IMIMAGE))
		capList << i18n("DirectIM/IMImage");
	if (hasCap(CAP_SENDBUDDYLIST))
		capList << i18n("Send Buddylist");
	if (hasCap(CAP_SENDFILE))
		capList << i18n("Send Files");
	if (hasCap(CAP_GETFILE))
		capList << i18n("Receive Files");

	if (capList.count() > 0)
		setProperty(mProtocol->clientFeatures, capList.join(", "));


	if(u.userclass & CLASS_AWAY)
	{
		if((this != account()->myself()) &&
		(account()->myself()->onlineStatus().status() != KopeteOnlineStatus::Connecting))
		{
			// request away message
			mAccount->engine()->sendUserLocationInfoRequest(contactName(),
				AIM_LOCINFO_AWAYMESSAGE);
		}
		setStatus(OSCAR_AWAY);
	}
	else
	{
		setStatus(OSCAR_ONLINE);
	}
}

void AIMContact::slotOffgoingBuddy(QString sn)
{
	if(tocNormalize(sn) != contactName())
		return;

	removeProperty(mProtocol->clientFeatures);
	removeProperty(mProtocol->awayMessage);

	setStatus(OSCAR_OFFLINE);
}

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
	finalMessage.replace("&nbsp;", " ");

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

	if( onlineStatus().status() == KopeteOnlineStatus::Offline )
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

const QString AIMContact::awayMessage()
{
	return property(mProtocol->awayMessage).value().toString();
}

void AIMContact::setAwayMessage(const QString &message)
{
	/*kdDebug(14150) << k_funcinfo <<
		"Called for '" << displayName() << "', away msg='" << message << "'" << endl;*/
	setProperty(mProtocol->awayMessage, message);
	emit awayMessageChanged();
}

#include "aimcontact.moc"
