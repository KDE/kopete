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

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>

#include "kopetestdaction.h"

#include "aimprotocol.h"
#include "aimaccount.h"
#include "aimcontact.h"
#include "aimuserinfo.h"
#include "aimchangestatus.h"

class KopeteMetaContact;

AIMAccount::AIMAccount(KopeteProtocol *parent, QString accountID, const char *name)
	: OscarAccount(parent, accountID, name, false)
{
	kdDebug(14190) << k_funcinfo << endl;
	mMyself = new AIMContact( accountID, accountID, this, 0L );
	// Instantiate the away dialog
	mAwayDialog = new AIMChangeStatus(getEngine());
}

AIMAccount::~AIMAccount()
{
	delete mAwayDialog;
}

OscarContact *AIMAccount::createNewContact( const QString &contactId,
		const QString &displayName, KopeteMetaContact *parentContact )
{
	return new AIMContact( contactId, displayName, this, parentContact );
}

KActionMenu* AIMAccount::actionMenu()
{
	// mActionMenu is managed by KopeteAccount.  It is deleted when
	// it is no longer shown, so we can (safely) just make a new one here.
	KActionMenu *mActionMenu =
		new KActionMenu(accountId(), this, "OscarAccount::mActionMenu");

	kdDebug(14190) << k_funcinfo << "for AIM account '"
				<< accountId() << "'" << endl;

	mActionMenu->popupMenu()->insertTitle(
			mMyself->onlineStatus().iconFor( mMyself ),
			i18n( "%2 <%1>" ).arg(accountId()).arg(mMyself->displayName()) );

	mActionMenu->insert(
		new KAction(i18n("Go O&nline"),
					AIMProtocol::protocol()->statusOnline.iconFor( this ),
					0, this, SLOT(slotGoOnline()), mActionMenu,
					"mActionGoOnline"));

	mActionMenu->insert(
		new KAction(i18n("Go &Offline"),
					AIMProtocol::protocol()->statusOffline.iconFor( this ),
					0, this, SLOT(slotGoOffline()), mActionMenu,
					"mActionGoOffline"));

	mActionMenu->insert(
		new KAction(i18n("Go &Away"),
					AIMProtocol::protocol()->statusAway.iconFor( this ),
					0, this, SLOT(slotGoAway()), mActionMenu,
					"mActionGoAway"));

	mActionMenu->popupMenu()->insertSeparator();

	mActionMenu->insert(
		KopeteStdAction::contactInfo(this, SLOT(slotEditInfo()),
									mActionMenu,
									"mActionEditInfo"));


	mActionMenu->popupMenu()->insertSeparator();

	mActionMenu->insert(
		new KAction( i18n("Show Debug"), "wizard", 0, this,
					 SLOT(slotShowDebugDialog()), mActionMenu,
					 "actionShowDebug") );

	mActionMenu->insert(
		new KAction(i18n("Fast add a Contact"), "", 0, this,
					SLOT(slotFastAddContact()), mActionMenu,
					"actionFastAddContact" ) );

	return mActionMenu;
}

void AIMAccount::initSignals()
{
	// Got my user info
	QObject::connect( getEngine(), SIGNAL(gotMyUserInfo(UserInfo)),
					  this, SLOT(slotGotMyUserInfo(UserInfo)));

	// Got warning
	QObject::connect( getEngine(),
		SIGNAL(gotWarning(int,QString)),
		this, SLOT(slotGotWarning(int,QString)));
}

void AIMAccount::slotGotMyUserInfo(UserInfo newInfo)
{
	mUserInfo = newInfo;
}

// FIXME: Called from AIMUserInfo
void AIMAccount::setUserProfile(QString profile)
{
	// Tell the engine to set the profile
	getEngine()->setMyProfile( profile );
	// Save the user profile
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
		{
			warnMessage = i18n("anonymously");
		}
		else
		{
			warnMessage = i18n("...warned by...", "by %1").arg(warner);
		}

		// Construct the message to be shown to the user
		QString message =
			i18n("You have been warned %1. Your new warning level is %2%.").arg(
				warnMessage).arg(newlevel);

		KMessageBox::sorry(0L,message);
	}
	mUserInfo.evil = newlevel;
}

void AIMAccount::slotEditInfo()
{
	AIMUserInfo *myInfo;

	myInfo = new AIMUserInfo(
		accountId(), accountId(),
		this, getEngine()->getMyProfile());

	myInfo->exec(); // This is a modal dialog
}

void AIMAccount::setAway(bool away, const QString &awayReason)
{
	kdDebug(14180) << k_funcinfo << " " << accountId() << "] setAway()" << endl;

	if(away)
		mEngine->sendAway(true, awayReason);
	else
		mEngine->sendAway(false, QString::null);
}



#include "aimaccount.moc"
