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

#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>

#include "kopeteawayaction.h"
#include "kopetestdaction.h"
#include "kopeteuiglobal.h"

#include "aim.h"

#include "aimprotocol.h"
#include "aimaccount.h"
#include "aimcontact.h"
#include "aimuserinfo.h"

class KopeteMetaContact;

AIMAccount::AIMAccount(KopeteProtocol *parent, QString accountID, const char *name)
	: OscarAccount(parent, accountID, name, false)
{
	kdDebug(14190) << k_funcinfo << accountID << ": Called."<< endl;
	mStatus = OSCAR_OFFLINE;

	setMyself( new AIMContact(tocNormalize(accountID), accountID, this, 0L) );
}

AIMAccount::~AIMAccount()
{
	kdDebug(14190) << k_funcinfo << "for '" << accountId() << "' deleted" << endl;
}

void AIMAccount::loaded()
{
	kdDebug(14190) << k_funcinfo << "Called." << endl;

	QString profile = pluginData(protocol(), "Profile");
	if(profile.isNull())
	{
		profile = QString::fromLocal8Bit("Visit the Kopete website at " \
			"<a href=\"http://kopete.kde.org\">http://kopete.kde.org</a>");
	}
	static_cast<AIMContact *>(myself())->setOwnProfile(profile);
}

OscarContact *AIMAccount::createNewContact( const QString &contactId,
		const QString &displayName, KopeteMetaContact *parentContact )
{
	return new AIMContact(contactId, displayName, this, parentContact);
}

KActionMenu* AIMAccount::actionMenu()
{
//	kdDebug(14190) << k_funcinfo << accountId() << ": Called." << endl;
	// mActionMenu is managed by KopeteAccount.  It is deleted when
	// it is no longer shown, so we can (safely) just make a new one here.
	KActionMenu *mActionMenu = new KActionMenu(accountId(),
		myself()->onlineStatus().iconFor( this ), this, "AIMAccount::mActionMenu");

	AIMProtocol *p = AIMProtocol::protocol();

	mActionMenu->popupMenu()->insertTitle(
		myself()->onlineStatus().iconFor(myself()),
		i18n("%2 <%1>")
#if QT_VERSION < 0x030200
			.arg(accountId()).arg(myself()->displayName()));
#else
			.arg(accountId(), myself()->displayName()));
#endif

	mActionMenu->insert(
		new KAction(p->statusOnline.caption(),
			p->statusOnline.iconFor(this), 0,
			this, SLOT(slotGoOnline()), mActionMenu,
			"AIMAccount::mActionOnline"));

	mActionMenu->insert(
		new KopeteAwayAction(p->statusAway.caption(),
		p->statusAway.iconFor(this), 0, 
		this, SLOT(slotGoAway( const QString & )), this, "AIMAccount::mActionNA" ) );
		
	KAction* mActionOffline = new KAction(p->statusOffline.caption(),
		p->statusOffline.iconFor(this),
		0, this, SLOT(slotGoOffline()), mActionMenu,
		"AIMAccount::mActionOffline");
	mActionOffline->setEnabled(isConnected());

	mActionMenu->insert(mActionOffline);
	mActionMenu->popupMenu()->insertSeparator();

	mActionMenu->insert(
		KopeteStdAction::contactInfo(this, SLOT(slotEditInfo()),
			mActionMenu, "AIMAccount::mActionEditInfo"));
	mActionOffline->setEnabled(isConnected());

	return mActionMenu;
}

void AIMAccount::initSignals()
{
	// Got my user info
	/*QObject::connect(
		engine(), SIGNAL(gotMyUserInfo(UserInfo &)),
		this, SLOT(slotGotMyUserInfo(UserInfo &)));*/

	// Got warning
	QObject::connect(
		engine(), SIGNAL(gotWarning(int,QString)),
		this, SLOT(slotGotWarning(int,QString)));
}

/*void AIMAccount::slotGotMyUserInfo(UserInfo &newInfo)
{
	kdDebug(14190) << k_funcinfo << "Called" << endl;
	mUserInfo = newInfo;
}*/

void AIMAccount::setUserProfile(const QString &profile)
{
	kdDebug(14190) << k_funcinfo << "called." << endl;
	static_cast<AIMContact *>(myself())->setOwnProfile(profile);
	setPluginData(protocol(), "Profile", profile);
}

// Called when we have been warned
void AIMAccount::slotGotWarning(int newlevel, QString warner)
{
	kdDebug(14190) << k_funcinfo << "Called." << endl;

	//this is not a natural decrease in level
	if (static_cast<AIMContact *>( myself() )->userInfo().evil < newlevel)
	{
		QString warnMessage;
		if(warner.isNull())
			warnMessage = i18n("anonymously");
		else
			warnMessage = i18n("...warned by...", "by %1").arg(warner);

#if QT_VERSION < 0x030200
		KMessageBox::sorry(Kopete::UI::Global::mainWidget(),
			i18n("You have been warned %1. Your new warning level is %2%.").arg(
				warnMessage).arg(newlevel));
#else
		KMessageBox::sorry(Kopete::UI::Global::mainWidget(),
			i18n("You have been warned %1. Your new warning level is %2%.").arg(
				warnMessage, newlevel));
#endif
	}

	// FIXME: How does this evil thing work?
	// We cannot set UserInfo in our myself contact right now!
	//mUserInfo.evil = newlevel;
}

void AIMAccount::slotEditInfo()
{
	AIMUserInfoDialog *myInfo = new AIMUserInfoDialog(static_cast<AIMContact *>(myself()), this,
	true, 0L, "myInfo");
	myInfo->exec(); // This is a modal dialog
}

void AIMAccount::setAway(bool away, const QString &awayReason)
{
	kdDebug(14190) << k_funcinfo << accountId() << ": setAway()" << endl;

	if(away)
		setStatus(OSCAR_AWAY, awayReason);
	else
		setStatus(OSCAR_ONLINE, QString::null);
}


void AIMAccount::setStatus(const unsigned long status,
	const QString &awayMessage)
{
	kdDebug(14190) << k_funcinfo << "new status=" << status <<
		", old status=" << mStatus << endl;
	mStatus = status;

	if(!awayMessage.isNull())
		setAwayMessage(awayMessage);

	if (isConnected())
		engine()->sendAIMAway((status==OSCAR_AWAY), awayMessage);
	else
		AIMAccount::connect(status, awayMessage);
}

void AIMAccount::slotGoOnline()
{
	if(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
	{
		kdDebug(14190) << k_funcinfo << "'" << accountId() <<
			"' was AWAY, marking back" << endl;
		setStatus(OSCAR_ONLINE, QString::null);
	}
	else if(myself()->onlineStatus().status() == KopeteOnlineStatus::Offline)
	{
		kdDebug(14190) << k_funcinfo << "'" << accountId() <<
			"' was OFFLINE, now connecting" << endl;
		AIMAccount::connect();
	}
	else
	{
		kdDebug(14190) << k_funcinfo << "'" << accountId() <<
			"' Already ONLINE" << endl;
	}
}

void AIMAccount::connect()
{
	kdDebug(14190) << k_funcinfo << "accountId='" << accountId() << "'" << endl;
	setStatus(OSCAR_ONLINE, QString::null);
}

void AIMAccount::connect(const unsigned long status, const QString &awMessage)
{
	kdDebug(14190) << k_funcinfo << "accountId='" << accountId() <<
		"', status=" << status << ", awaymessage=" << awMessage << endl;

	// Get the screen name for this account
	QString screenName = accountId();
	QString server = pluginData(protocol(), "Server");
	QString port = pluginData(protocol(), "Port");

	if(server.isEmpty())
	{
		slotError(i18n("You have not specified a server address in the " \
			"account set up yet, please do so."), 0);
	}
	else if(port.isEmpty() || (port.toInt() < 1))
	{
		slotError(i18n("You have not specified a server port in the " \
			"account set up yet, please do so."), 0);
	}
	else if (screenName != i18n("(No Screen Name Set)") ) // FIXME: Is this needed at all?
	{
		QString _password = password(false, 0L, 16);
		if (_password.isEmpty())
		{
			slotError(i18n("Kopete is unable to attempt to sign-on to the " \
				"AIM network because no password was specified in the " \
				"preferences."), 0);
		}
		else
		{
			kdDebug(14190) << k_funcinfo << accountId() <<
				": Logging in as " << screenName << endl;

			// Connect, need to normalize the name first
			engine()->doLogin(
				server,
				port.toInt(),
				screenName,
				_password,
				static_cast<AIMContact *>(myself())->userProfile(),
				status,
				awMessage);
		}
	}
	else
	{
		slotError(i18n("You have not specified your account name in the " \
			"account set up yet, please do so."), 0);
	}
}

void AIMAccount::slotGoAway(const QString &message)
{
	setAway(true,message);
}

#include "aimaccount.moc"
// vim: set noet ts=4 sts=4 sw=4:
