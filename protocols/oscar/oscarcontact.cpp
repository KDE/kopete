/*
  oscarcontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
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

#include "oscarcontact.h"

#include <time.h>

#include <qapplication.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"

#include "aim.h"
#include "oscarsocket.h"
#include "oscaraccount.h"

OscarContact::OscarContact(const QString& name, const QString& displayName,
	KopeteAccount *account, KopeteMetaContact *parent)
	: KopeteContact(account, name, parent)
{
// 	kdDebug(14150) << k_funcinfo "name='" << name <<
// 		"', displayName='" << displayName << "'"<< endl;

	if (!account)
		kdDebug(14150) << k_funcinfo << "Account pointer was null!" << endl;
	else
		mAccount = static_cast<OscarAccount*>(account);

	mName = tocNormalize(name); // We store normalized names (lowercase no spaces)
	mMsgManager = 0L;
	mIdle = 0;
	mRealIP = 0;
	mLocalIP = 0;
	mPort = 0;
	mFwType = 0;
	mTcpVersion = 0;

	mListContact = mAccount->internalBuddyList()->findBuddy(mName); // TODO: remove AIMBuddy

	if (!mListContact) // this Contact is not yet in the internal contactlist!
	{
		mListContact = new AIMBuddy(mAccount->randomNewBuddyNum(), 0, mName);
		mAccount->internalBuddyList()->addBuddy(mListContact);
	}

	setFileCapable(true); // FIXME: depends on status!

	if (!displayName.isEmpty())
		setDisplayName(displayName);
	else
		setDisplayName(name);

	initSignals();
}

void OscarContact::initSignals()
{
//	kdDebug(14150) << k_funcinfo << "Called" << endl;
	// Buddy offline
	QObject::connect(
		mAccount->engine(), SIGNAL(gotOffgoingBuddy(QString)),
		this, SLOT(slotOffgoingBuddy(QString)));

	// kopete-users's status changed
	QObject::connect(
		mAccount->engine(), SIGNAL(statusChanged(const unsigned int)),
		this, SLOT(slotMainStatusChanged(const unsigned int)));

	QObject::connect(
		mAccount->engine(), SIGNAL(gotBuddyChange(const UserInfo &)),
		this, SLOT(slotParseUserInfo(const UserInfo &)));
	// Got IM
/*
	QObject::connect(
		mAccount->engine(), SIGNAL(gotIM(QString,QString,bool)),
		this, SLOT(slotIMReceived(QString,QString,bool)));
*/
	// New direct connection
	QObject::connect(
		mAccount->engine(), SIGNAL(connectionReady(QString)),
		this, SLOT(slotDirectIMReady(QString)));
	// Direct connection closed
	QObject::connect(
		mAccount->engine(), SIGNAL(directIMConnectionClosed(QString)),
		this, SLOT(slotDirectIMConnectionClosed(QString)));
	// File transfer request
	QObject::connect(
		mAccount->engine(), SIGNAL(gotFileSendRequest(QString,QString,QString,unsigned long)),
		this, SLOT(slotGotFileSendRequest(QString,QString,QString,unsigned long)));
	// File transfer started
	QObject::connect(
		mAccount->engine(), SIGNAL(transferBegun(OscarConnection *, const QString &,
			const unsigned long, const QString &)),
		this, SLOT(slotTransferBegun(OscarConnection *,
			const QString &,
			const unsigned long,
			const QString &)));
	// File transfer manager stuff
	QObject::connect(
		KopeteTransferManager::transferManager(), SIGNAL(accepted(KopeteTransfer *, const QString &)),
				this, SLOT(slotTransferAccepted(KopeteTransfer *, const QString &)) );
	// When the file transfer is refused
	QObject::connect(
		KopeteTransferManager::transferManager(), SIGNAL(refused(const KopeteFileTransferInfo &)),
		this, SLOT(slotTransferDenied(const KopeteFileTransferInfo &)));

	// When a group in the contact list is being removed, we're notified
	QObject::connect(
		KopeteContactList::contactList(), SIGNAL(groupRemoved(KopeteGroup*)),
			this, SLOT(slotGroupRemoved(KopeteGroup *)));

//	kdDebug(14150) << "[OscarContact] Finished initializing signal connections" << endl;
}

OscarContact::~OscarContact()
{
//	kdDebug(14150) << k_funcinfo << "Called for '" << mName << "'" << endl;

	// FIXME: Why was this here, we shouldn't try to remove users from
	// contactlist just because their destructor was called
//	slotDeleteContact();
}


KopeteMessageManager* OscarContact::manager(bool canCreate)
{
	if(!mMsgManager && canCreate)
	{
		QPtrList<KopeteContact> theContact;
		theContact.append(this);

		mMsgManager = KopeteMessageManagerFactory::factory()->create(account()->myself(), theContact, protocol());

		// This is for when the user types a message and presses send
		QObject::connect(mMsgManager, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager *)),
			this, SLOT(slotSendMsg(KopeteMessage&, KopeteMessageManager *)));
		// For when the message manager is destroyed
		QObject::connect(mMsgManager, SIGNAL(destroyed()), this, SLOT(slotMessageManagerDestroyed()));
	}
	return mMsgManager;

}

void OscarContact::slotMessageManagerDestroyed()
{
	mMsgManager = 0L;
}

// TODO: Can be removed when AIMBuddy is gone
void OscarContact::slotUpdateBuddy()
{
	// FIXME: just to make sure the stupid AIMBuddy has proper status
	// This should be handled in AIM/ICQContact now!
	mListContact->setStatus(onlineStatus().internalStatus());

	if (mAccount->isConnected())
	{
		// Probably already broken. Does not work at all for ICQ because uin never changes
		if (mName != mListContact->screenname()) // contact changed his nickname
		{
			if(!mListContact->alias().isEmpty())
				setDisplayName(mListContact->alias());
			else
				setDisplayName(mListContact->screenname());
		}
	}
	else // oscar-account is offline so all users are offline too
	{
		mListContact->setStatus(OSCAR_OFFLINE); // TODO: remove AIMBuddy
		setStatus(OSCAR_OFFLINE);
	}
}

void OscarContact::slotMainStatusChanged(const unsigned int newStatus)
{
	if(newStatus == OSCAR_OFFLINE)
	{
		setStatus(OSCAR_OFFLINE);
		slotUpdateBuddy();
	}
}

void OscarContact::slotOffgoingBuddy(QString sn)
{
	if(tocNormalize(sn) == mName) //if we are the contact that is offgoing
	{
		setStatus(OSCAR_OFFLINE);
		slotUpdateBuddy();
	}
}

void OscarContact::slotUpdateNickname(const QString newNickname)
{
	setDisplayName(newNickname);
	//emit updateNickname ( newNickname );
	mListContact->setAlias(newNickname); // TODO: remove AIMBuddy :)
}

void OscarContact::slotDeleteContact()
{
	kdDebug(14150) << k_funcinfo << "contact '" << displayName() << "'" << endl;
	AIMGroup *group = mAccount->internalBuddyList()->findGroup(mListContact->groupID());
	if (!group)
		return;

	mAccount->internalBuddyList()->removeBuddy(mListContact);
	mAccount->engine()->sendDelBuddy(mListContact->screenname(),group->name());
	deleteLater();
}

void OscarContact::slotGroupRemoved(KopeteGroup * /* removedGroup */ )
{
/******
//For an unknown reason, when deleting a group, even if the contact is not in this group, the contact was deleted.
//In many case, when a group is deleted, Kopete move contacts to toplevel. and OscarContact::syncGroups() is called
// so you can handle the moving to top level here  (it should be called before the deletion of the group)

	AIMGroup *aGroup = mAccount->internalBuddyList()->findGroup(mListContact->groupID());
	if (!aGroup)
	{
		kdDebug(14150) << k_funcinfo << "contact '" << displayName() <<
			"' has no AIMGroup associated with it!" << endl;
		return;
	}

	if (aGroup->name() != removedGroup->displayName()) // our group did not get removed
		return;

	kdDebug(14150) << k_funcinfo << "displayName=" << displayName() <<
		", groupname=" << aGroup->name() <<
		", Calling slotDeleteContact()" << endl;

	slotDeleteContact();

*****/
}

void OscarContact::slotBlock()
{
	QString message = i18n("<qt>Are you sure you want to block %1?" \
		" Once blocked, this user will no longer be visible to you. The block can be" \
		" removed later in the preferences dialog.</qt>").arg(mName);

	int result = KMessageBox::questionYesNo(
		qApp->mainWidget(),
		message,
		i18n("Block User %1?").arg(mName),
		i18n("Block"));

	if (result == KMessageBox::Yes)
		mAccount->engine()->sendBlock(mName);
}

void OscarContact::slotDirectConnect()
{
	kdDebug(14150) << k_funcinfo << "Requesting direct IM with " << mName << endl;

	int result = KMessageBox::questionYesNo(
		qApp->mainWidget(),
		i18n("<qt>Are you sure you want to establish a direct connection to %1? \
		This will allow %2 to know your IP address, which can be dangerous if \
		you do not trust this contact.</qt>")
#if QT_VERSION < 0x030200
			.arg(mName).arg(mName),
#else
			.arg(mName , mName),
#endif
		i18n("Request Direct IM with %1?").arg(mName));
	if(result == KMessageBox::Yes)
	{
		execute();
		KopeteContactPtrList p;
		p.append(this);
		KopeteMessage msg = KopeteMessage(
			this, p,
			i18n("Waiting for %1 to connect...").arg(mName),
			KopeteMessage::Internal, KopeteMessage::PlainText );

		manager()->appendMessage(msg);
		mAccount->engine()->sendDirectIMRequest(mName);
	}
}

void OscarContact::slotDirectIMReady(QString name)
{
	// Check if we're the one who is directly connected
	if(tocNormalize(name) != mName)
		return;

	kdDebug(14150) << "[OscarContact] Setting direct connect state for "
				   << mName << " to true." << endl;

	mDirectlyConnected = true;
	KopeteContactPtrList p;
	p.append(this);
	KopeteMessage msg = KopeteMessage(
		this, p,
		i18n("Direct connection to %1 established").arg(mName),
		KopeteMessage::Internal, KopeteMessage::PlainText ) ;

	manager()->appendMessage(msg);
}

/** Called when the direct connection to contact @name has been terminated */
void OscarContact::slotDirectIMConnectionClosed(QString name)
{
	// Check if we're the one who is directly connected
	if ( tocNormalize(name) != tocNormalize(mName) )
		return;

	kdDebug(14150) << "[OscarContact] Setting direct connect state for '"
		<< mName << "' to false." << endl;

	mDirectlyConnected = false;
}

void OscarContact::sendFile(const KURL &sourceURL, const QString &/*altFileName*/,
	const long unsigned int /*fileSize*/)
{
	KURL filePath;

	//If the file location is null, then get it from a file open dialog
	if( !sourceURL.isValid() )
		filePath = KFileDialog::getOpenURL(QString::null ,"*", 0L, i18n("Kopete File Transfer"));
	else
		filePath = sourceURL;

	if(!filePath.isEmpty())
	{
		KFileItem finfo(KFileItem::Unknown, KFileItem::Unknown, filePath);

		kdDebug(14150) << k_funcinfo << "File size is " <<
			(unsigned long)finfo.size() << endl;

		//Send the file
		mAccount->engine()->sendFileSendRequest(mName, finfo);
	}
}

// Called when the metacontact owning this contact has changed groups
void OscarContact::syncGroups()
{
	// Log the function entry
//	kdDebug(14150) << k_funcinfo << ": Entering" << endl;
	// Get the new (kopete) group that we belong to
	KopeteGroupList groups = metaContact()->groups();
	// Oscar only supports one group per contact, so just get the first one
	KopeteGroup *newKopeteGroup = groups.first();
	if (newKopeteGroup == 0L)
	{
		kdDebug(14150) << k_funcinfo << ": Could not get kopete group" << endl;
		return;
	}

	kdDebug(14150) << k_funcinfo << ": Getting current oscar group " << mListContact->groupID() << " ... " << endl;
	// Get the current (oscar) group that this contact belongs to on the server
	AIMGroup *currentOscarGroup =
		mAccount->internalBuddyList()->findGroup(mListContact->groupID());
	if (currentOscarGroup == 0L)
	{
		kdDebug(14150) << k_funcinfo << ": Could not get current Oscar group "
					   << "for contact" << endl;
		return;
	}

	kdDebug(14150) << k_funcinfo << ": Current oscar group id: " <<
		mListContact->groupID() << ", Current oscar group name: " <<
		currentOscarGroup->name() << endl;

	// Compare the two names, to see if they're actually different
	if (currentOscarGroup->name() != newKopeteGroup->displayName())
	{ // First check to see if the new group is actually on the server list yet
		AIMGroup *newOscarGroup =
			mAccount->internalBuddyList()->findGroup(newKopeteGroup->displayName());
		if (newOscarGroup == 0L)
		{ // This is a new group, it doesn't exist on the server yet
			kdDebug(14150) << k_funcinfo
				<< ": New group did not exist on server, "
				<< "asking server to create it first"
				<< endl;
			// Ask the server to create the group
			mAccount->engine()->sendAddGroup(newKopeteGroup->displayName());
		}

		// The group has changed, so ask the engine to change
		// our group on the server
		mAccount->engine()->sendChangeBuddyGroup(
			tocNormalize(mListContact->screenname()),
			currentOscarGroup->name(),
			newKopeteGroup->displayName());
	}
}

void OscarContact::slotGotFileSendRequest(QString sn, QString message, QString filename,
	unsigned long filesize)
{
	if(tocNormalize(sn) != mName)
		return;

	kdDebug(14150) << k_funcinfo << "Got file transfer request for '" <<
		displayName() << "'" << endl;

	KopeteTransferManager::transferManager()->askIncomingTransfer(
		this, filename, filesize, message);
}

void OscarContact::slotTransferAccepted(KopeteTransfer *tr, const QString &fileName)
{
	if (tr->info().contact() != this)
		return;

	kdDebug(14150) << k_funcinfo << "Transfer of '" << fileName <<
		"' from '" << mName << "' accepted." << endl;

	OscarConnection *fs = mAccount->engine()->sendFileSendAccept(mName, fileName);

	//connect to transfer manager
	QObject::connect(
		fs, SIGNAL(percentComplete(unsigned int)),
		tr, SLOT(slotPercentCompleted(unsigned int)));
}

void OscarContact::slotTransferDenied(const KopeteFileTransferInfo &tr)
{
	// Check if we're the one who is directly connected
	if(tr.contact() != this)
		return;

	kdDebug(14150) << k_funcinfo << "Transfer denied." << endl;
	mAccount->engine()->sendFileSendDeny(mName);
}

void OscarContact::slotTransferBegun(OscarConnection *con,
	const QString& file,
	const unsigned long size,
	const QString &recipient)
{
	if (tocNormalize(con->connectionName()) != mName)
		return;

	kdDebug(14150) << k_funcinfo << "adding transfer of " << file << endl;
	KopeteTransfer *tr = KopeteTransferManager::transferManager()->addTransfer(
		this, file, size, recipient, KopeteFileTransferInfo::Outgoing );

	//connect to transfer manager
	QObject::connect(
		con, SIGNAL(percentComplete(unsigned int)),
		tr, SLOT(slotPercentCompleted(unsigned int)));
}

void OscarContact::rename(const QString &newNick)
{
	kdDebug(14150) << k_funcinfo << "Rename '" << displayName() << "' to '" <<
		newNick << "'" << endl;

	if(mAccount->isConnected())
	{
		//FIXME: group handling!
		AIMGroup *currentOscarGroup =
			mAccount->internalBuddyList()->findGroup(mListContact->groupID());
		if(currentOscarGroup)
		{
			mAccount->engine()->sendRenameBuddy(mName,
				currentOscarGroup->name(), newNick);
		}
		else
		{
			kdDebug(14150) << k_funcinfo <<
				"couldn't find AIMGroup for contact, can't rename on server" << endl;
		}
	}

	mListContact->setAlias(newNick);
	setDisplayName(newNick);
}

void OscarContact::slotParseUserInfo(const UserInfo &u)
{
	if(tocNormalize(u.sn) != mName)
		return;

	mRealIP = u.realip;
	mLocalIP = u.localip;
	mPort = u.port;
	mFwType = u.fwType;
	mTcpVersion = u.version;
	if (mIdle != u.idletime)
	{
		mIdle = u.idletime;
		if(mIdle > 0)
			setIdleState(Idle);
		else // we are not idling anymore
			setIdleState(Active);
	}
	mSignonTime.setTime_t(u.onlinesince);
}

#include "oscarcontact.moc"
// vim: set noet ts=4 sts=4 sw=4:
