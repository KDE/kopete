/*
  icqontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Stefan Gehn  <metz AT gehn.net>
  Copyright (c) 2003 by Olivier Goffart
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

#include "icqcontact.h"

#include "icqprotocol.h"
#include "icqaccount.h"

//#include <time.h>

#include <qapplication.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopetemessagemanagerfactory.h"

#include "icquserinfo.h"
static const unsigned int SUPPORTED_INFO_ITEMS = 7;

ICQContact::ICQContact(const QString name, const QString displayName,
	ICQAccount *acc, KopeteMetaContact *parent)
	: OscarContact(name, displayName, acc, parent)
{
	mProtocol = static_cast<ICQProtocol *>(protocol());

	mInvisible = false;
	setOnlineStatus(mProtocol->statusOffline);

	infoDialog = 0L;
	userinfoRequestSequence=0;
	userinfoReplyCount = 0;
	generalInfo.uin=0;
	generalInfo.nickName="";
	generalInfo.firstName="";
	generalInfo.lastName="";
	generalInfo.eMail="";
	generalInfo.city="";
	generalInfo.state="";
	generalInfo.phoneNumber="";
	generalInfo.faxNumber="";
	generalInfo.street="";
	generalInfo.cellularNumber="";
	generalInfo.zip="";
	generalInfo.countryCode=0;
	generalInfo.timezoneCode=0;
	generalInfo.publishEmail=false;
	generalInfo.showOnWeb=false;

	workInfo.city="";
	workInfo.state="";
	workInfo.phone="";
	workInfo.fax="";
	workInfo.address="";
	workInfo.zip="";
	workInfo.countryCode=0;
	workInfo.company="";
	workInfo.department="";
	workInfo.position="";
	workInfo.occupation=0;
	workInfo.homepage="";

	/*if(name == account()->accountId())
	{
		QObject::connect(
			acc->engine(), SIGNAL(gotMyUserInfo(const UserInfo &)),
			this, SLOT(slotContactChanged(const UserInfo &)));
	}
	else*/
	{
		// Buddy Changed
		QObject::connect(
			acc->engine(), SIGNAL(gotContactChange(const UserInfo &)),
			this, SLOT(slotContactChanged(const UserInfo &)));
	}

/*	QObject::connect(
		acc->engine(), SIGNAL(gotIM(QString &,QString &,bool)),
		this, SLOT(slotGotIM(QString &,QString &,bool)));*/
	QObject::connect(
		acc->engine(), SIGNAL(gotICQGeneralUserInfo(const int, const ICQGeneralUserInfo &)),
		this, SLOT(slotUpdGeneralInfo(const int, const ICQGeneralUserInfo &)));
	QObject::connect(
		acc->engine(), SIGNAL(gotICQWorkUserInfo(const int, const ICQWorkUserInfo &)),
		this, SLOT(slotUpdWorkInfo(const int, const ICQWorkUserInfo &)));
	QObject::connect(
		acc->engine(), SIGNAL(gotICQMoreUserInfo(const int, const ICQMoreUserInfo &)),
		this, SLOT(slotUpdMoreUserInfo(const int, const ICQMoreUserInfo &)));
	QObject::connect(
		acc->engine(), SIGNAL(gotICQAboutUserInfo(const int, const QString &)),
		this, SLOT(slotUpdAboutUserInfo(const int, const QString &)));
	QObject::connect(
		acc->engine(), SIGNAL(gotICQEmailUserInfo(const int, const ICQMailList &)),
		this, SLOT(slotUpdEmailUserInfo(const int, const ICQMailList &)));
	QObject::connect(
		acc->engine(), SIGNAL(gotICQInfoItemList(const int, const ICQInfoItemList &)),
		this, SLOT(slotUpdInterestUserInfo(const int, const ICQInfoItemList &)));
	QObject::connect(
		acc->engine(), SIGNAL(gotICQInfoItemList(const int, const ICQInfoItemList &, const ICQInfoItemList & )),
		this, SLOT(slotUpdBackgroundUserInfo(const int, const ICQInfoItemList &, const ICQInfoItemList & ) ) );
}

ICQContact::~ICQContact()
{
}

void ICQContact::setOwnDisplayName(const QString &s)
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;
	if(this == account()->myself())
		setDisplayName(s);
}

void ICQContact::slotContactChanged(const UserInfo &u)
{
	if (u.sn != contactName())
		return;

	/*kdDebug(14190) << k_funcinfo << "Called for '"
		<< displayName() << "', contactName()=" << contactName() << endl;*/

	mInvisible = (u.icqextstatus & ICQ_STATUS_IS_INVIS);

	if (u.icqextstatus & ICQ_STATUS_IS_FFC)
		setStatus(OSCAR_FFC);
	else if (u.icqextstatus & ICQ_STATUS_IS_DND)
		setStatus(OSCAR_DND);
	else if (u.icqextstatus & ICQ_STATUS_IS_OCC)
		setStatus(OSCAR_OCC);
	else if (u.icqextstatus & ICQ_STATUS_IS_NA)
		setStatus(OSCAR_NA);
	else if (u.icqextstatus & ICQ_STATUS_IS_AWAY)
		setStatus(OSCAR_AWAY);
	else
		setStatus(OSCAR_ONLINE);

	slotUpdateBuddy();
}

void ICQContact::slotOffgoingBuddy(QString sender)
{
	if(sender != contactName())
		return;

	setOnlineStatus(mProtocol->statusOffline);
	slotUpdateBuddy();
}

void ICQContact::gotIM(OscarSocket::OscarMessageType /*type*/, const QString &message)
{
	// Build a KopeteMessage and set the body as Rich Text
	KopeteContactPtrList tmpList;
	tmpList.append(account()->myself());
	KopeteMessage msg(this, tmpList, message, KopeteMessage::Inbound,
		KopeteMessage::RichText);
	manager(true)->appendMessage(msg);
}

void ICQContact::slotSendMsg(KopeteMessage& message, KopeteMessageManager *)
{
	if (message.plainBody().isEmpty()) // no text, do nothing
		return;

	// Check to see if we're even online
	if(!account()->isConnected())
	{
		KMessageBox::sorry(qApp->mainWidget(),
			i18n("<qt>You must be logged on to ICQ before you can "
				"send a message to a user.</qt>"),
			i18n("Not Signed On"));
		return;
	}

	// FIXME: We don't do HTML in ICQ
	// we might be able to do that in AIM and we might also convert
	// HTML to RTF for ICQ type-2 messages  [mETz]
	static_cast<OscarAccount*>(account())->engine()->sendIM(
		message.plainBody(), this, false);

	// Show the message we just sent in the chat window
	manager()->appendMessage(message);
	manager()->messageSucceeded();
}

bool ICQContact::isReachable()
{
	return true;
}

KActionCollection *ICQContact::customContextMenuActions()
{
	actionCollection = new KActionCollection(this);

	KAction* actionRequestAuth = new KAction(i18n("&Request Authorization"), 0,
		this, SLOT(slotRequestAuth()), actionCollection, "actionRequestAuth");
	KAction* actionSendAuth = new KAction(i18n("&Send Authorization"), 0,
		this, SLOT(slotSendAuth()), actionCollection, "actionSendAuth");

	QString awTxt;
	QString awIcn;
	int status = onlineStatus().internalStatus();
	if (status >= 15)
		status-=15; // get rid of invis addon
	switch(status)
	{
		case OSCAR_FFC:
			awTxt = i18n("Read Free For Chat &Message");
			awIcn = "icq_ffc";
			break;
		case OSCAR_AWAY:
			awTxt = i18n("Read Away &Message");
			awIcn = "icq_away";
			break;
		case OSCAR_DND:
			awTxt = i18n("Read Do Not Disturb &Message");
			awIcn = "icq_dnd";
			break;
		case OSCAR_NA:
			awTxt = i18n("Read Not Available &Message");
			awIcn = "icq_na";
			break;
		case OSCAR_OCC:
			awTxt = i18n("Read Occupied &Message");
			awIcn = "icq_occ" ;
			break;
	}
	KAction* actionReadAwayMessage = new KAction(awTxt, awIcn, 0,
		this, SLOT(slotReadAwayMessage()), actionCollection, "actionReadAwayMessage");

	actionRequestAuth->setEnabled(waitAuth());

	actionCollection->insert(actionRequestAuth);
	actionCollection->insert(actionSendAuth);
	actionCollection->insert(actionReadAwayMessage);

	return actionCollection;
}

void ICQContact::setStatus(const unsigned int newStatus)
{
	if((onlineStatus().internalStatus() == newStatus) && !mInvisible)
		return;

	switch(newStatus)
	{
		case OSCAR_FFC:
			setOnlineStatus(mProtocol->statusFFC);
			break;
		case OSCAR_OFFLINE:
			mInvisible = false;
			setOnlineStatus(mProtocol->statusOffline);
			break;
		case OSCAR_AWAY:
			setOnlineStatus(mProtocol->statusAway);
			break;
		case OSCAR_DND:
			setOnlineStatus(mProtocol->statusDND);
			break;
		case OSCAR_NA:
			setOnlineStatus(mProtocol->statusNA);
			break;
		case OSCAR_OCC:
			setOnlineStatus(mProtocol->statusOCC);
			break;
		case OSCAR_CONNECTING:
			setOnlineStatus(mProtocol->statusConnecting); // ONLY for myself() contact
			break;
		default: // emergency choose, also OSCAR_ONLINE
			setOnlineStatus(mProtocol->statusOnline);
	}
}

void ICQContact::setOnlineStatus(const KopeteOnlineStatus& status)
{
	if(mInvisible)
	{
		kdDebug(14200) << k_funcinfo << "'" << displayName() << "' is invisible!" << endl;
		KopeteContact::setOnlineStatus(
			KopeteOnlineStatus(
				status.status() , (status.weight()==0) ? 0 : (status.weight() -1),
				protocol(),
				status.internalStatus()+15,
				QString::fromLatin1("icq_invisible"),
				status.caption(),
				i18n("%1|Invisible").arg(status.description())
				)
			);
	}
	else
	{
		KopeteContact::setOnlineStatus(status);
	}
/*
	kdDebug(14200) << k_funcinfo << "'" << displayName() << "' is now " <<
		onlineStatus().description() << endl;
*/
}


void ICQContact::slotUserInfo()
{
	if (!infoDialog)
	{
		infoDialog = new ICQUserInfo(this, static_cast<ICQAccount*>(account()));
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

void ICQContact::slotCloseUserInfoDialog()
{
	infoDialog->delayedDestruct();
	infoDialog = 0L;
}

void ICQContact::requestUserInfo()
{
	kdDebug(14200) << k_funcinfo << "called" << endl;
	userinfoReplyCount = 0;
	userinfoRequestSequence =
		account()->engine()->sendReqInfo(contactName().toULong());
}

void ICQContact::slotUpdGeneralInfo(const int seq, const ICQGeneralUserInfo &inf)
{
	// compare reply's sequence with the one we sent with our last request
	if(seq != userinfoRequestSequence)
		return;

// 	kdDebug(14200) << k_funcinfo << "called; seq=" << seq << ", last saved seq=" <<
// 		userinfoRequestSequence << endl;

	generalInfo = inf;

	userinfoReplyCount++;
	if (userinfoReplyCount >= SUPPORTED_INFO_ITEMS)
		emit updatedUserInfo();
}

void ICQContact::slotUpdWorkInfo(const int seq, const ICQWorkUserInfo &inf)
{
	// compare reply's sequence with the one we sent with our last request
	if(seq != userinfoRequestSequence)
		return;

// 	kdDebug(14200) << k_funcinfo << "called; seq=" << seq << ", last saved seq=" <<
// 		userinfoRequestSequence << endl;

	workInfo = inf;

	userinfoReplyCount++;
	if (userinfoReplyCount >= SUPPORTED_INFO_ITEMS)
		emit updatedUserInfo();
}

void ICQContact::slotUpdMoreUserInfo(const int seq, const ICQMoreUserInfo &inf)
{
	// compare reply's sequence with the one we sent with our last request
	if(seq != userinfoRequestSequence)
		return;

// 	kdDebug(14200) << k_funcinfo << "called; seq=" << seq << ", last saved seq=" <<
// 		userinfoRequestSequence << endl;

	moreInfo = inf;

	userinfoReplyCount++;
	if (userinfoReplyCount >= SUPPORTED_INFO_ITEMS)
		emit updatedUserInfo();
}

void ICQContact::slotUpdAboutUserInfo(const int seq, const QString &inf)
{
	// compare reply's sequence with the one we sent with our last request
	if(seq != userinfoRequestSequence)
		return;

//  	kdDebug(14200) << k_funcinfo << "called; seq=" << seq << ", last saved seq=" <<
//  		userinfoRequestSequence << endl;

	aboutInfo = inf;

	userinfoReplyCount++;
	if (userinfoReplyCount >= SUPPORTED_INFO_ITEMS)
		emit updatedUserInfo();
}

void ICQContact::slotUpdEmailUserInfo(const int seq, const ICQMailList &inf)
{
	// compare reply's sequence with the one we sent with our last request
	if(seq != userinfoRequestSequence)
		return;

// 	kdDebug(14200) << k_funcinfo << "called; seq=" << seq << ", last saved seq=" <<
// 		userinfoRequestSequence << endl;

	emailInfo = inf;

	userinfoReplyCount++;
	if (userinfoReplyCount >= SUPPORTED_INFO_ITEMS)
		emit updatedUserInfo();
}

void ICQContact::slotUpdInterestUserInfo(const int seq, const ICQInfoItemList &inf)
{
	// compare reply's sequence with the one we sent with our last request
	if(seq != userinfoRequestSequence)
		return;
	interestInfo = inf;
	userinfoReplyCount++;
	if (userinfoReplyCount >= SUPPORTED_INFO_ITEMS)
		emit updatedUserInfo();
}

void ICQContact::slotUpdBackgroundUserInfo(const int seq, const ICQInfoItemList &curr, const ICQInfoItemList &past)
{
	// compare reply's sequence with the one we sent with our last request
	if(seq != userinfoRequestSequence)
		return;
	currentBackground = curr;
	pastBackground = past;
	userinfoReplyCount++;
	if (userinfoReplyCount >= SUPPORTED_INFO_ITEMS)
		emit updatedUserInfo();
}

void ICQContact::slotReadAwayMessage()
{
	kdDebug(14200) << k_funcinfo << endl;
	account()->engine()->requestAwayMessage(this);
}

#include "icqcontact.moc"
// vim: set noet ts=4 sts=4 sw=4:
