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
//#include <kfiledialog.h>

#include <kdeversion.h>
#if KDE_IS_VERSION( 3, 1, 90 )
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"

#include "aim.h"
#include "oscarsocket.h"
#include "oscaraccount.h"

#include <assert.h>

OscarContact::OscarContact(const QString& name, const QString& displayName,
	KopeteAccount *account, KopeteMetaContact *parent)
	: KopeteContact(account, name, parent)
{
	/*kdDebug(14150) << k_funcinfo <<
		"name='" << name <<
		"', displayName='" << displayName << "'" << endl;*/

	assert(account);

	mAccount = static_cast<OscarAccount*>(account);

	mName = tocNormalize(name); // We store normalized names (lowercase no spaces)
	mEncoding=0;
	mGroupId=0;
	mMsgManager=0L;

	// BEGIN TODO: remove AIMBuddy
	mListContact=mAccount->internalBuddyList()->findBuddy(mName);

	if (!mListContact) // this Contact is not yet in the internal contactlist!
	{
		mListContact=new AIMBuddy(mAccount->randomNewBuddyNum(), 0, mName);
		mAccount->internalBuddyList()->addBuddy(mListContact);
	}
	// END TODO: remove AIMBuddy

	setFileCapable(false); // FIXME

	if (!displayName.isEmpty())
		setDisplayName(displayName);
	else
		setDisplayName(name);

	// fill userinfo with default values until we receive a userinfo block from the contact
	mInfo.sn = name;
	mInfo.capabilities = 0;
	mInfo.icqextstatus = ICQ_STATUS_OFFLINE;

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
		mAccount->engine(), SIGNAL(gotContactChange(const UserInfo &)),
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
/*
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
*/
	// When a group in the contact list is being removed, we're notified
	QObject::connect(
		KopeteContactList::contactList(), SIGNAL(groupRemoved(KopeteGroup*)),
			this, SLOT(slotGroupRemoved(KopeteGroup *)));

	QObject::connect(
		mAccount->engine(), SIGNAL(gotAuthReply(const QString &, const QString &, bool)),
		this, SLOT(slotGotAuthReply(const QString &, const QString &, bool)));
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
		mInfo.idletime = 0;
		setIdleTime(0);
		emit idleStateChanged(this);
	}
}

void OscarContact::slotMainStatusChanged(const unsigned int newStatus)
{
	if(newStatus == OSCAR_OFFLINE)
	{
		setStatus(OSCAR_OFFLINE);
		slotUpdateBuddy();
		mInfo.idletime = 0;
		setIdleTime(0);
		emit idleStateChanged(this);
	}
}

void OscarContact::slotOffgoingBuddy(QString sn)
{
	if(tocNormalize(sn) == mName) //if we are the contact that is offgoing
	{
		setStatus(OSCAR_OFFLINE);
		slotUpdateBuddy();
		mInfo.idletime = 0;
		setIdleTime(0);
		emit idleStateChanged(this);
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

	AIMGroup *group = mAccount->internalBuddyList()->findGroup(mGroupId);

	if(!group && metaContact() && metaContact()->groups().count() > 0)
	{
		QString grpName=metaContact()->groups().first()->displayName();
		kdDebug(14150) << k_funcinfo <<
			"searching group by name '" << grpName << "'" << endl;
		group=mAccount->internalBuddyList()->findGroup(grpName);
	}

	if (!group)
	{
		kdDebug(14150) << k_funcinfo <<
			"Couldn't find serverside group for contact, cannot delete on server :(" << endl;
		return;
	}
	else
	{
		mAccount->engine()->sendDelBuddy(contactName(), group->name());
	}

	mAccount->internalBuddyList()->removeBuddy(mListContact);
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
	if(tocNormalize(name) != contactName())
		return;

	kdDebug(14150) << k_funcinfo << "Setting direct connect state for '" <<
		displayName() << "' to true" << endl;

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
#if 0
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
#endif
// Called when the metacontact owning this contact has changed groups
void OscarContact::syncGroups()
{
	// Get the (kopete) group that we belong to
	KopeteGroupList groups = metaContact()->groups();
	if(groups.count() == 0)
	{
		kdDebug(14150) << k_funcinfo << "Contact is in no Group in Kopete Contactlist, aborting" << endl;
		return;
	}

	// Oscar only supports one group per contact, so just get the first one
	KopeteGroup *firstKopeteGroup = groups.first();
	if(!firstKopeteGroup)
	{
		kdDebug(14150) << k_funcinfo << "Could not get kopete group" << endl;
		return;
	}

//	kdDebug(14150) << k_funcinfo << ": Getting current oscar group " << mListContact->groupID() << " ... " << endl;
	// Get the current (oscar) group that this contact belongs to on the server
	AIMGroup *currentOscarGroup = mAccount->internalBuddyList()->findGroup(mGroupId);
	if (!currentOscarGroup)
	{
		kdDebug(14150) << k_funcinfo <<
			"Could not get current Oscar group for contact '" << displayName() <<
			"'" << endl;
		return;
	}

	kdDebug(14150) << k_funcinfo <<
		"Current OSCAR group id=" << mGroupId <<
		", Current OSCAR group name='" << currentOscarGroup->name() << "'" << endl;

	// Compare the two names, to see if they're actually different
	if (currentOscarGroup->name() != firstKopeteGroup->displayName())
	{
		// First check to see if the new group is actually on the server list yet
		AIMGroup *newOscarGroup =
			mAccount->internalBuddyList()->findGroup(firstKopeteGroup->displayName());

		if(!newOscarGroup)
		{
			// This is a new group, it doesn't exist on the server yet
			kdDebug(14150) << k_funcinfo
				<< ": New group did not exist on server, "
				<< "asking server to create it first"
				<< endl;
			// Ask the server to create the group
			mAccount->engine()->sendAddGroup(firstKopeteGroup->displayName());
		}

		// TODO: update groupID in OscarContact

		// The group has changed, so ask the engine to change
		// our group on the server
		mAccount->engine()->sendChangeBuddyGroup(
			contactName(),
			currentOscarGroup->name(),
			firstKopeteGroup->displayName());
	}
}

#if 0
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
#endif

void OscarContact::rename(const QString &newNick)
{
	kdDebug(14150) << k_funcinfo << "Rename '" << displayName() << "' to '" <<
		newNick << "'" << endl;

	AIMGroup *currentOscarGroup = 0L;

	if(mAccount->isConnected())
	{
		//FIXME: group handling!
		currentOscarGroup =
			mAccount->internalBuddyList()->findGroup(mGroupId);
		if(!currentOscarGroup)
		{
			// FIXME: workaround for not knowing the groupid
			if(metaContact() && metaContact()->groups().count() > 0)
			{
				QString grpName=metaContact()->groups().first()->displayName();
				kdDebug(14150) << k_funcinfo <<
					"searching group by name '" << grpName << "'" << endl;
				currentOscarGroup=mAccount->internalBuddyList()->findGroup(grpName);
			}
		}

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

	mListContact->setAlias(newNick); // FIXME: remove AIMBuddy
	setDisplayName(newNick);
}

void OscarContact::slotParseUserInfo(const UserInfo &u)
{
	if(tocNormalize(u.sn) != contactName())
		return;

	if (mInfo.idletime != u.idletime)
	{
		setIdleTime(u.idletime*60);
		/*if(mIdle > 0)
			setIdleState(Idle);
		else // we are not idling anymore
			setIdleState(Active);*/
		if(u.idletime == 0)
			emit idleStateChanged(this);
	}

	mInfo = u;
	/*kdDebug(14150) << k_funcinfo << "Contact '" << displayName() <<
		"', mInfo.sn=" << mInfo.sn << ", u.sn=" << u.sn << endl;

	if(mInfo.capabilities & AIM_CAPS_UTF8)
		kdDebug(14150) << k_funcinfo << "Contact '" << displayName() << "' announced UTF support!" << endl;*/
}

void OscarContact::slotRequestAuth()
{
	kdDebug(14150) << k_funcinfo << "Called for '" << displayName() << "'" << endl;

#if KDE_IS_VERSION( 3, 1, 90 )
	QString reason = KInputDialog::getText(
		i18n("Request Authorization"),i18n("Reason for requesting authorization"));
#else
	QString reason = KLineEditDlg::getText(
		i18n("Request Authorization"),i18n("Reason for requesting authorization"));
#endif
	if(!reason.isNull())
	{
		kdDebug(14150) << k_funcinfo << "Sending auth request to '" <<
			displayName() << "'" << endl;
		mAccount->engine()->sendAuthRequest(contactName(), reason);
	}
}

void OscarContact::slotSendAuth()
{
	kdDebug(14150) << k_funcinfo << "Called for '" << displayName() << "'" << endl;

	// TODO: custom dialog also allowing a refusal
#if KDE_IS_VERSION( 3, 1, 90 )
	QString reason = KInputDialog::getText(
		i18n("Request Authorization"),i18n("Reason for requesting authorization"));
#else
	QString reason = KLineEditDlg::getText(
		i18n("Grant Authorization"),i18n("Reason for granting authorization"));
#endif
	if(!reason.isNull())
	{
		kdDebug(14150) << k_funcinfo << "Sending auth granted to '" <<
			displayName() << "'" << endl;
		mAccount->engine()->sendAuthReply(contactName(), reason, true);
	}
}

void OscarContact::slotGotAuthReply(const QString &contact, const QString &reason, bool granted)
{
	if(contact != contactName())
		return;

	kdDebug(14150) << k_funcinfo << "Called for '" << displayName() << "' reason='" <<
		reason << "' granted=" << granted << endl;

	setWaitAuth(granted);
	if(granted)
	{
		QString message = i18n("<b>[Granted Authorization:]</b> %1").arg(reason);
		gotIM(OscarSocket::GrantedAuth, message);
	}
	else
	{
		QString message = i18n("<b>[Declined Authorization:]</b> %1").arg(reason);
		gotIM(OscarSocket::DeclinedAuth, message);
	}
}

bool OscarContact::waitAuth() const
{
	// TODO: move var to OscarContact
	return mListContact->waitAuth();
}

void OscarContact::setWaitAuth(bool b) const
{
	mListContact->setWaitAuth(b);
}

const unsigned int OscarContact::encoding()
{
	return mEncoding;
}

void OscarContact::setEncoding(const unsigned int mib)
{
	mEncoding = mib;
}

const int OscarContact::groupId()
{
	kdDebug(14150) << k_funcinfo << "returning" << mGroupId << endl;
	return mGroupId;
}

void OscarContact::setGroupId(const int newgid)
{
	if(newgid > 0)
	{
		mGroupId = newgid;
		//kdDebug(14150) << k_funcinfo << "updated group id to " << mGroupId << endl;
	}
}

void OscarContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &/*addressBookData*/)
{
	serializedData["awaitingAuth"] = waitAuth() ? "1" : "0";
	serializedData["Encoding"] = QString::number(mEncoding);
	serializedData["groupID"] = QString::number(mGroupId);
}

#include "oscarcontact.moc"
// vim: set noet ts=4 sts=4 sw=4:
