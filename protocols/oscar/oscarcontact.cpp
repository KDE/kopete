/***************************************************************************
                          oscarcontact.cpp  -  description
                             -------------------
    begin                : Tue Jul 30 2002
    copyright            : (C) 2002 by twl6
    email                : twl6@po.cwru.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "oscarcontact.h"
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include "kopete.h"
#include "kopetestdaction.h"
#include "kopetewindow.h"
#include "kopeteaway.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetehistorydialog.h"
#include "oscarsocket.h"
#include "oscaruserinfo.h"
#include "oscarprotocol.h"
#include "aim.h"

OscarContact::OscarContact(const QString name, OscarProtocol *protocol,
		KopeteMetaContact *parent) : KopeteContact(protocol->id(), parent)
{
	mName = name;
	mProtocol = protocol;
	mMsgManager = 0L;
	historyDialog = 0L;
	QObject::connect(mProtocol->engine, SIGNAL(gotOncomingBuddy(UserInfo)),this,SLOT(slotOncomingBuddy(UserInfo)));
	QObject::connect(mProtocol->engine, SIGNAL(gotOffgoingBuddy(QString)),this,SLOT(slotOffgoingBuddy(QString)));
	QObject::connect(mProtocol->engine, SIGNAL(gotIM(QString,QString,bool)),this,SLOT(slotIMReceived(QString,QString,bool)));
	initActions();
	TBuddy tmpBuddy;
	int num = mProtocol->buddyList()->getNum(mName);
	if ( mProtocol->buddyList()->get(&tmpBuddy, num) != -1 )
	{
		if ( !tmpBuddy.alias.isEmpty() )
			setDisplayName(tmpBuddy.alias);
		else
			setDisplayName(tmpBuddy.name);

		slotBuddyChanged(num);
	}
	else
	{
		setDisplayName(mName);
	}
	theContacts.append(this);
}

OscarContact::~OscarContact()
{
}

/** Pops up a chat window */
void OscarContact::execute(void)
{
	kdDebug() << "[OscarContact] execute()" << endl;

	if ( mStatus == OSCAR_OFFLINE )
	{
		KMessageBox::sorry(kopeteapp->mainWindow(), i18n(
			"<qt>Sorry, this user is not online at the moment for you" \
			" to message him/her. AIM users must be online for you to be" \
			" able to message them.</qt>"), i18n("User not Online") );
		return;
	}
	msgManager()->readMessages();
}

/** Show a context menu of actions pertaining to this contact */
/*void OscarContact::showContextMenu(const QPoint &p, const QString &group)
{
	KPopupMenu *popup = new KPopupMenu();

	popup->insertTitle( QString("%1 (%2)").arg(mName).arg(statusText()) );

	actionSendMessage->plug( popup );
	popup->insertSeparator();

	actionInfo->plug( popup );
	actionViewHistory->plug( popup );
	popup->insertSeparator();

	actionAddGroup->plug( popup );
	actionRemove->plug( popup );

	actionWarn->plug( popup ); 

	popup->exec( p );

	delete popup;
} */

/** Return the unique id that identifies a contact.  Id is required
 *  to be unique per protocol and per identity.  Across those boundaries
 *  ids may occur multiple times. */
QString OscarContact::id(void) const
{
	return mName;
}

/** Return the protocol specific serialized data that a plugin may want to store a contact list. */
QString OscarContact::data(void) const
{
	return QString::null;
}

KopeteMessageManager* OscarContact::msgManager()
{
	if ( mMsgManager )
	{
		//printf("REturning a mmsgmanager: %d\n",mMsgManager);fflush(stdout);
		return mMsgManager;
	}
	else
	{
		//printf("Creating a mmsgmanager: %d\n",mProtocol->myself());fflush(stdout);
		mMsgManager = kopeteapp->sessionFactory()->create(mProtocol->myself(), theContacts, mProtocol, "aim_logs/" + mName +".log", KopeteMessageManager::ChatWindow);
		connect(mMsgManager, SIGNAL(messageSent(const KopeteMessage&, KopeteMessageManager *)), this, SLOT(slotSendMsg(const KopeteMessage&, KopeteMessageManager *)));
		return mMsgManager;
	}
}

/** Called when a buddy changes */
void OscarContact::slotBuddyChanged(int buddyNum)
{
	TBuddy tmpBuddy;

	// buddy not found in our list of buddies
	if ( mProtocol->buddyList()->get(&tmpBuddy, buddyNum) == -1 )
		return;

	QString tmpBuddyName = tocNormalize ( tmpBuddy.name );

	if ( tmpBuddyName != tocNormalize(mName) ) // that's not our contact
		return;

	// status did not change, do nothing
	if ( mStatus == tmpBuddy.status )
		return;

	mStatus = tmpBuddy.status;

	kdDebug() << "[OscarContact] slotBuddyChanged(), Contact " << mName << " is now " << mStatus << endl;

	if ( mProtocol->isConnected() ) // oscar-plugin is online
	{
		if ( mName != tmpBuddyName ) // contact changed his nickname
		{
			if ( !tmpBuddy.alias.isEmpty() )
				setDisplayName(tmpBuddy.alias);
			else
				setDisplayName(tmpBuddy.name);
		}
	}
	else // oscar-plugin is offline so all users are offline too
	{
		mStatus = OSCAR_OFFLINE;
		mProtocol->buddyList()->setStatus(buddyNum, OSCAR_OFFLINE);

//		actionSendMessage->setEnabled(false);
//		actionInfo->setEnabled(false);

//		emit userStatusChanged(OSCAR_OFFLINE);
//		emit statusChanged();
		emit statusChanged( this, status() );
		return;
	}


	// We can only send messages to online users
//	actionSendMessage->setEnabled(mStatus != TAIM_OFFLINE);
//	actionInfo->setEnabled(mStatus != TAIM_OFFLINE);

	//emit userStatusChanged(tmpBuddy.status);
	//emit statusChanged();
	emit statusChanged( this, status() );
}

/** Returns the online status of the contact */
OscarContact::ContactStatus OscarContact::status(void) const
{
	if (mStatus == OSCAR_ONLINE)
		return Online;
	else if (mStatus == OSCAR_AWAY)
		return Away;
	else
		return Offline;
}

/** Initialzes the actions */
void OscarContact::initActions(void)
{
	actionCollection = 0L;

	actionWarn = new KAction(i18n("&Warn"), 0, this, SLOT(slotWarn()), this, "actionWarn");
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
void OscarContact::slotOncomingBuddy(UserInfo u)
{
	if (tocNormalize(u.sn) == tocNormalize(mName))
	//if we are teh contact that is oncoming
	{
		TBuddy *tmpBuddy;
		int num = mProtocol->buddyList()->getNum(mName);
		kdDebug() << "[OscarContact] Names match... " << u.sn << endl;
		if ( (tmpBuddy = mProtocol->buddyList()->getByNum(num)) != NULL )
		{
			kdDebug() << "[OscarContact] Setting status for " << u.sn << endl;
			if ( u.userclass & USERCLASS_AWAY )
				tmpBuddy->status = OSCAR_AWAY;
			else
				tmpBuddy->status = OSCAR_ONLINE;
			slotBuddyChanged(num);
		}
		else
			kdDebug() << "[OscarContact] Buddy is oncoming but is not in buddy list\n";
	}
}

/** Called when a buddy is offgoing */
void OscarContact::slotOffgoingBuddy(QString sn)
{
	if (tocNormalize(sn) == tocNormalize(mName))
	//if we are the contact that is offgoing
	{
		TBuddy *tmpBuddy;
		int num = mProtocol->buddyList()->getNum(mName);
		if ( (tmpBuddy = mProtocol->buddyList()->getByNum(num)) != NULL )
		{
			tmpBuddy->status = OSCAR_OFFLINE;
			slotBuddyChanged(num);
		}
		else
			kdDebug() << "[OscarContact] Buddy is offgoing but not in buddy list\n";
	}
}

/** Called when user info is requested */
void OscarContact::slotUserInfo(void)
{
	TBuddy tmpBuddy;
	int num = mProtocol->buddyList()->getNum(mName);

	if (mProtocol->buddyList()->get(&tmpBuddy, num) != -1)
	{
		if (!mProtocol->isConnected())
		{
			KMessageBox::sorry(kopeteapp->mainWindow(), i18n("<qt>Sorry, you must be connected to the AIM server to retrieve user information, but you will be allowed to continue if you would like to change the user's nickname.</qt>"), i18n("You Must be Connected") );
		} else {
			if (tmpBuddy.status == TAIM_OFFLINE)
			{
				KMessageBox::sorry(kopeteapp->mainWindow(), i18n("<qt>Sorry, this user isn't online for you to view his/her information, but you will be allowed to only change his/her nickname. Please wait until this user becomes available and try again</qt>" ), i18n("User not Online"));
			}
		}
		OscarUserInfo *Oscaruserinfo = new OscarUserInfo(mName, tmpBuddy.alias, mProtocol, tmpBuddy);
		connect(Oscaruserinfo, SIGNAL(updateNickname(const QString)), this, SLOT(slotUpdateNickname(const QString)));
		Oscaruserinfo->show();
	}
}

/** Called when an IM is received */
void OscarContact::slotIMReceived(QString message, QString sender, bool /*isAuto*/)
{
	if ( tocNormalize(sender) != tocNormalize(mName) )
		return;

	TBuddy tmpBuddy;
	mProtocol->buddyList()->get(&tmpBuddy, mProtocol->buddyList()->getNum(mName));

	KopeteContactPtrList tmpList;
	tmpList.append(mProtocol->myself());
	KopeteMessage msg ( this, tmpList, message, KopeteMessage::Inbound);
	msgManager()->appendMessage(msg);
	if ( mProtocol->isAway() ) // send our away message in fire-and-forget-mode :)
	{
		// TODO: move to aimprefs and add gui in there!
		KGlobal::config()->setGroup("Oscar");
		QString reply = KGlobal::config()->readEntry("AwayMessage", QString("I'm currently away from my computer. Please leave a message for me when I return to my computer."));

		if ( KopeteAway::globalAway() )
			reply = KopeteAway::message();

		mProtocol->engine->sendIM(reply, mName, true);
		KopeteMessage replymsg ( mProtocol->myself(), theContacts , reply, KopeteMessage::Outbound);
		msgManager()->appendMessage(replymsg);
	}
}

/** Called when we want to send a message */
void OscarContact::slotSendMsg(const KopeteMessage& message, KopeteMessageManager *)
{
	if ( message.body().isEmpty() ) // no text, do nothing
		return;

	TBuddy *tmpBuddy = mProtocol->buddyList()->getByNum(mProtocol->buddyList()->getNum(mName));

	if (!mProtocol->isConnected())
	{
		KMessageBox::sorry(kopeteapp->mainWindow(), i18n("<qt>You must be logged on to AIM before you can send a message to a user.</qt>"), i18n("Not Signed On"));
		return;
	}
	if (tmpBuddy->status == TAIM_OFFLINE || mStatus == TAIM_OFFLINE)
	{
		KMessageBox::sorry(kopeteapp->mainWindow(), i18n("<qt>This user is not online at the moment for you to message him/her. AIM users must be online for you to be able to message them.</qt>"), i18n("User not Online"));
		return;
	}
	QString msg = message.body();

	if ( message.fg().isValid() ) // we want a custom foreground-color
		msg.prepend ( QString("<FONT COLOR=\"%1\">").arg(message.fg().name()) );
	else
		msg.prepend ( QString("<FONT>") );

	msg.append ( "</FONT>" );

	if ( message.fg().isValid() ) // we want a custom background-color
		msg.prepend ( QString("<HTML><BODY BGCOLOR=\"%1\">").arg(message.bg().name()) );
	else
		msg.prepend ( QString("<HTML><BODY>") );

	msg.append ( "</BODY></HTML>" );
	mProtocol->engine->sendIM( msg, mName, false );
	KopeteMessage kopetemsg ( mProtocol->myself(), theContacts , message.body(), KopeteMessage::Outbound);
	kopetemsg.setFg(message.fg());
	kopetemsg.setBg(message.bg());
	kopetemsg.setFont(message.font());
	msgManager()->appendMessage(kopetemsg);
}

/** Called when nickname needs to be updated */
void OscarContact::slotUpdateNickname(const QString newNickname)
{
	setDisplayName( newNickname );
	//emit updateNickname ( newNickname );

	TBuddy *tmp;
	tmp = mProtocol->buddyList()->getByNum(mProtocol->buddyList()->getNum(mName));
	tmp->alias = newNickname;
}

/** View the history dialog */
void OscarContact::slotViewHistory(void)
{
	if (historyDialog != 0L)
	{
		historyDialog->raise();
	}
	else
	{
		historyDialog = new KopeteHistoryDialog(QString("oscar_logs/%1.log").arg(mName), mName, true, 50, 0, "OscarHistoryDialog");

		connect ( historyDialog, SIGNAL(closing()), this, SLOT(slotCloseHistoryDialog()) );
	}
}

/** Called when history dialog is closed */
void OscarContact::slotCloseHistoryDialog(void)
{
	delete historyDialog;
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
	return actionCollection;
}

/** Method to delete a contact from the contact list */
void OscarContact::slotDeleteContact(void)
{
	TBuddy tmpBuddy;
	mProtocol->buddyList()->get( &tmpBuddy, mProtocol->buddyList()->getNum(mName) );
	QString buddyName = (tmpBuddy.alias.isEmpty() ? mName : tmpBuddy.alias);

	if (
		KMessageBox::warningYesNo(
			kopeteapp->mainWindow(),
			i18n("<qt>Are you sure you want to remove %1 from your contact list?</qt>").arg(buddyName),
			i18n("Confirmation")
			) == KMessageBox::Yes )
	{
		mProtocol->buddyList()->del(tocNormalize(mName));
		mProtocol->engine->sendDelBuddy(tmpBuddy.name,mProtocol->buddyList()->getNameGroup(tmpBuddy.group));
		delete this;
	}
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
	QString title = i18n("Warn User %1?").arg(mName);

	int result = KMessageBox::questionYesNoCancel(kopeteapp->mainWindow(), message, title);
	if (result == KMessageBox::Yes)
	{
		mProtocol->engine->sendWarning(mName, true);
	}
	else if (result == KMessageBox::No)
	{
		mProtocol->engine->sendWarning(mName, false);
	}
}

#include "oscarcontact.moc"
