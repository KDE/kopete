//
//
// C++ Implementation: cpp
//
// Description:
//
//
// Author: Duncan Mac-Vicar Prett <duncan@kde.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>

#include "kopetestdaction.h"

#include "oscarchangestatus.h"
#include "aimprotocol.h"
#include "aimaccount.h"
#include "aimcontact.h"
#include "aimuserinfo.h"

class KopeteMetaContact;

AIMAccount::AIMAccount(KopeteProtocol *parent, QString accountID, const char *name)
	: OscarAccount(parent, accountID, name, false)
{
	kdDebug(14190) << k_funcinfo << accountID << ": Called."<< endl;
	mAwayMessage = QString::null;
	mStatus = OSCAR_OFFLINE;

	mMyself = new AIMContact(accountID, accountID, this, 0L);
	QObject::connect(mAwayDialog, SIGNAL(goAway(const int, const QString&)),
		this, SLOT(slotAwayDialogReturned(const int, const QString&)));
}

AIMAccount::~AIMAccount()
{
	kdDebug(14190) << k_funcinfo << "[" << accountId() << "] deleted" << endl;
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
	static_cast<AIMContact *>(mMyself)->setOwnProfile(profile);
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
		mMyself->onlineStatus().iconFor(mMyself),
		i18n("%2 <%1>")
#if QT_VERSION < 0x030200
			.arg(accountId()).arg(mMyself->displayName()));
#else
			.arg(accountId(), mMyself->displayName()));
#endif

	mActionMenu->insert(
		new KAction(p->statusOnline.caption(),
			p->statusOnline.iconFor(this), 0,
			this, SLOT(slotGoOnline()), mActionMenu,
			"AIMAccount::mActionOnline"));

	mActionMenu->insert(
		new KAction(p->statusAway.caption(),
			p->statusAway.iconFor(this), 0,
			this, SLOT(slotGoAway()), mActionMenu,
			"AIMAccount::mActionAway"));

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
	QObject::connect(
		engine(), SIGNAL(gotMyUserInfo(UserInfo &)),
		this, SLOT(slotGotMyUserInfo(UserInfo &)));

	// Got warning
	QObject::connect(
		engine(), SIGNAL(gotWarning(int,QString)),
		this, SLOT(slotGotWarning(int,QString)));
}

void AIMAccount::slotGotMyUserInfo(UserInfo &newInfo)
{
	mUserInfo = newInfo;
}

void AIMAccount::setUserProfile(const QString &profile)
{
	kdDebug(14190) << k_funcinfo << "called." << endl;
	static_cast<AIMContact *>(mMyself)->setOwnProfile(profile);
	setPluginData(protocol(), "Profile", profile);
}

// Called when we have been warned
void AIMAccount::slotGotWarning(int newlevel, QString warner)
{
	kdDebug(14190) << k_funcinfo << "Called." << endl;

	//this is not a natural decrease in level
	if (mUserInfo.evil < newlevel)
	{
		QString warnMessage;
		if(warner.isNull())
			warnMessage = i18n("anonymously");
		else
			warnMessage = i18n("...warned by...", "by %1").arg(warner);

#if QT_VERSION < 0x030200
		KMessageBox::sorry(0L,
			i18n("You have been warned %1. Your new warning level is %2%.").arg(
				warnMessage).arg(newlevel));
#else
		KMessageBox::sorry(0L,
			i18n("You have been warned %1. Your new warning level is %2%.").arg(
				warnMessage, newlevel));
#endif
	}
	mUserInfo.evil = newlevel;
}

void AIMAccount::slotEditInfo()
{
	AIMUserInfoDialog *myInfo = new AIMUserInfoDialog(static_cast<AIMContact *>(mMyself), this,
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
		mAwayMessage = awayMessage;

	if (isConnected())
		engine()->sendAIMAway((status==OSCAR_AWAY), awayMessage);
	else
		AIMAccount::connect(status, awayMessage);
}

void AIMAccount::slotGoOnline()
{
	if(mMyself->onlineStatus().status() == KopeteOnlineStatus::Away)
	{
		kdDebug(14190) << k_funcinfo << "'" << accountId() <<
			"' was AWAY, marking back" << endl;
		setStatus(OSCAR_ONLINE, QString::null);
	}
	else if(mMyself->onlineStatus().status() == KopeteOnlineStatus::Offline)
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
		QString password = getPassword();
		if (password.isEmpty())
		{
			slotError(i18n("Kopete is unable to attempt to signon to the " \
				"ICQ network because no password was specified in the " \
				"preferences."), 0);
		}
		else
		{
			kdDebug(14190) << k_funcinfo << accountId() <<
				": Logging in as " << screenName << endl;

			// Connect, need to normalize the name first
			mEngine->doLogin(
				server,
				port.toInt(),
				screenName,
				password,
				static_cast<AIMContact *>(mMyself)->userProfile(),
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

void AIMAccount::slotAwayDialogReturned(const int status, const QString &message)
{
	setStatus(status,message);
}

#include "aimaccount.moc"
// vim: set noet ts=4 sts=4 sw=4:
