/***************************************************************************
                          jabbercontact.cpp  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Daniel Stone
    email                : dstone@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qcursor.h>
#include <qfont.h>
#include <qptrlist.h>

#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>

#include "dlgjabbervcard.h"
#include "dlgrename.h"
#include "jabbercontact.h"
#include "jabberprotocol.h"
#include "jabberresource.h"
#include "jabtasks.h"
#include "jabcommon.h"
#include "kopete.h"
#include "kopetestdaction.h"
#include "kopetewindow.h"
#include "kopetemessage.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "ui/kopetehistorydialog.h"

/*
 * JabberContact constructor
 */
JabberContact::JabberContact(QString userID, QString nickname, QString group, JabberProtocol *protocol, KopeteMetaContact *mc, QString identity) : KopeteContact(protocol->id(), mc)
{

	// save parent protocol object
	mProtocol = protocol;
	
	parentMetaContact = mc;
	
	// no resource so far
	hasResource = false;
	
	historyDialog = 0L;
	mMsgManagerKCW = 0L;
	mMsgManagerKEW = 0L;
	popup = 0L;
	
	mIdentityId = identity;

	// initialize contact with data
	initContact(userID, nickname, group);

}

JabberContact::~JabberContact()
{
	
	delete actionMessage;
	delete actionRemoveFromGroup;
	delete actionRename;
	delete actionSelectResource;
	delete actionSnarfVCard;

	// Authorization actions 
	delete actionSendAuth;
	delete actionRerequestAuth;

	if(popup)
		delete popup;

}

void JabberContact::initContact(QString &userID, QString &nickname, QString &group)
{

	// if the nickname is empty, we will override it
	// with the user ID and mark it as not using a local name
	if (nickname.isNull())
	{
		nickname = userID;
		hasLocalName = false;
	}
	else
	{
		hasLocalName = true;
	}

	// initialize protocol-specific variables
	mGroup = group;
	mUserID = userID;
	
	// determine if we have a local group
	// assigned to this contact
	if (mGroup == QString(""))
	{
		hasLocalGroup = false;
	}
	else
	{
		hasLocalGroup = true;
	}

	// create popup menu for the contact
	initActions();
	
	// update the displayed name
	setDisplayName(nickname);
	
	// specifically cause this instance to update this contact as offline
	slotUpdateContact("", STATUS_OFFLINE, "");
	
	theContacts.append(this);

}

KopeteMessageManager* JabberContact::msgManagerKEW()
{

	if (mMsgManagerKEW != 0L)
		// return current instance if there is already one
		return mMsgManagerKEW;
	else
	{
		// get new instance
		mMsgManagerKEW = kopeteapp->sessionFactory()->create(mProtocol->myself(), theContacts, mProtocol, "jabber_logs/" + mUserID +".log", KopeteMessageManager::Email);
		
		connect(mMsgManagerKEW, SIGNAL(messageSent(const KopeteMessage&, KopeteMessageManager*)),
			this, SLOT(slotSendMsgKEW(const KopeteMessage&)));
			
		return mMsgManagerKEW;
	}

}

KopeteMessageManager* JabberContact::msgManagerKCW()
{
	
	if (mMsgManagerKCW != 0L)
		// return current instance if there is already one
		return mMsgManagerKCW;
	else
	{
		// get new instance
		mMsgManagerKCW = kopeteapp->sessionFactory()->create(mProtocol->myself(), theContacts, mProtocol, "jabber_logs/" + mUserID +".log", KopeteMessageManager::ChatWindow);
		
		connect(mMsgManagerKCW, SIGNAL(messageSent(const KopeteMessage&, KopeteMessageManager*)),
			this, SLOT(slotSendMsgKCW(const KopeteMessage&)));

		return mMsgManagerKCW;
	}

}

void JabberContact::initActions()
{
    
	actionChat = KopeteStdAction::sendMessage(this, SLOT(slotChatThisUser()), this, "actionChat");
	actionMessage = new KAction(i18n("Send Email Message"), "mail_generic", 0, this, SLOT(slotEmailUser()), this, "actionMessage");
	actionRemoveFromGroup = new KAction(i18n("Remove From Group"), "edittrash", 0, this, SLOT(slotRemoveFromGroup()), this, "actionRemove");
	actionRemove = KopeteStdAction::deleteContact(this, SLOT(slotRemoveThisUser()), this, "actionDelete");
	actionContactMove = KopeteStdAction::moveContact(this, SLOT(slotMoveThisUser()), this, "actionMove");
	actionHistory = KopeteStdAction::viewHistory(this, SLOT(slotViewHistory()), this, "actionHistory");
	actionRename = new KAction(i18n("Rename Contact"), "editrename", 0, this, SLOT(slotRenameContact()), this, "actionRename");
	actionSelectResource = new KSelectAction(i18n("Select Resource"), "selectresource", 0, this, SLOT(slotSelectResource()), this, "actionSelectResource");
	actionSnarfVCard = new KAction(i18n("Get vCard"), "identity", 0, this, SLOT(slotSnarfVCard()), this, "actionSnarfVCard");

	// Authorization actions 
	actionSendAuth = new KAction(i18n("(Re)send authorization to"), "", 0, this, SLOT(slotSendAuth()), this, "actionSendAuth");
	actionRerequestAuth = new KAction(i18n("Rerequest authorization from"), "", 0, this, SLOT(slotSendAuth()), this, "actionRerequestAuth");

}

void JabberContact::showContextMenu(const QPoint&, const QString&)
{

	// if the popup menu has been instantiated before,
	// delete it now
	if(popup)
		delete popup;
	
	// create a new popup menu
	popup = new KPopupMenu();
	popup->insertTitle(userID() + " (" + mResource + ")");

	KGlobal::config()->setGroup("Jabber");
	
	// depending on the window type preference,
	// chat window or email window goes first
	if (KGlobal::config()->readBoolEntry("EmailDefault", false))
	{
		actionMessage->plug(popup);
		actionChat->plug(popup);
	}
	else
	{
		actionChat->plug(popup);
		actionMessage->plug(popup);
	}

	// if the contact is online,
	// display the resources we have for it
	if (mStatus != STATUS_OFFLINE)
	{
		QStringList items;
		int activeItem = 0;
		JabberResource *tmpBestResource = bestResource();
		
		// put best resource first
		items.append("Automatic (best resource)");
		items.append(tmpBestResource->resource());
		
		// iterate through available resources
		int i = 1;
		for (JabberResource *tmpResource = resources.first(); tmpResource; tmpResource = resources.next(), i++)
		{
			// only add the item if it is not the best resource
			// (which we already added above)
			if (tmpResource != tmpBestResource)
				items.append(tmpResource->resource());
				
			// mark the currently active resource
			if (hasResource && (tmpResource->resource() == activeResource->resource()))
			{
				kdDebug() << "[JabberContact] Woot woot, it's the same resource, activating(ish) item " << i << endl;
				activeItem = i;
			}
		}
		
		// attach list to the menu action
		actionSelectResource->setItems(items);
		
		// make sure the active item is selected
		if (!hasResource || !activeItem)
		{
			actionSelectResource->setCurrentItem(0);
		}
		else
		{
			actionSelectResource->setCurrentItem(activeItem);
			kdDebug() << "[JabberContact] Item " << activeItem << " fully activated." << endl;
		}
		
		// plug it to the menu
		actionSelectResource->plug(popup);
	}
	
	popup->insertSeparator();

	actionSnarfVCard->plug(popup);
	actionHistory->plug(popup);
	popup->insertSeparator();
	actionRename->plug(popup);
	actionContactMove->plug(popup);
	actionSendAuth->plug(popup);
	actionRerequestAuth->plug(popup);
	actionRemoveFromGroup->plug(popup);
	actionRemove->plug(popup);

	popup->popup(QCursor::pos());

}

void JabberContact::slotUpdateContact(QString resource, int newStatus, QString reason)
{
	
	kdDebug() << "[JabberContact] Updating contact " << mUserID << "  to " << newStatus << endl;

	if (newStatus != -1)
		mStatus = newStatus;

	if (resource != QString(""))
		mResource = resource;

	if (reason != QString(""))
		mReason = reason;

	emit statusChanged(this, status());

}

void JabberContact::slotRenameContact()
{

	kdDebug() << "[JabberContact] Renaming contact." << endl;

	dlgRename = new dlgJabberRename;

	dlgRename->lblUserID->setText(userID());
	
	if(hasLocalName)
		dlgRename->leNickname->setText(displayName());
	else
		dlgRename->leNickname->setText(userID());

	connect(dlgRename->btnRename, SIGNAL(clicked()), this, SLOT(slotDoRenameContact()));

	dlgRename->show();

}

void JabberContact::slotDoRenameContact()
{
	QString name = dlgRename->leNickname->text();
	
	// if the name has been deleted, revert
	// to using the user ID instead
	if (name == QString(""))
	{
		hasLocalName = false;
		name = mUserID;
	}
	else
	{
		hasLocalName = true;
	}

	// send rename request to protocol backend
	mProtocol->renameContact(userID(), hasLocalName ? name : QString(""), hasLocalGroup ? mGroup : QString(""));
	
	// update display
	setDisplayName(displayName());

	// delete dialog
	delete dlgRename;

}

void JabberContact::slotDeleteMySelf(bool)
{
    
	delete this;

}

JabberContact::ContactStatus JabberContact::status() const
{
	JabberContact::ContactStatus retval;

	switch(mStatus)
	{
		case STATUS_ONLINE:
					retval = Online;
					break;
		case STATUS_AWAY:
		case STATUS_XA:
		case STATUS_DND:
					retval = Away;
					break;
		default:
					retval = Offline;
					break;
	}
	
	return retval;
	
}

QString JabberContact::statusText() const
{
	QString txt;

	switch ( mStatus )
	{
		case STATUS_ONLINE:
			txt = i18n("Online");
			break;
		case STATUS_AWAY:
			txt = i18n("Away");
			break;
		case STATUS_XA:
			txt = i18n("Extended Away");
			break;
		case STATUS_DND:
			txt = i18n("Do Not Disturb");
			break;
		default:
			txt = i18n("Offline");
			break;
	}

	// append away reason if there is one
	if ( !mReason.isNull() && !mReason.isEmpty() )
		txt += " (" + mReason + ")";

	return txt;
}

QString JabberContact::statusIcon() const
{
	QString icon;
	
	switch(mStatus)
	{
		case STATUS_ONLINE:
					icon = "jabber_online";
					break;
		case STATUS_AWAY:
		case STATUS_XA:
					icon = "jabber_away";
					break;
		case STATUS_DND:
					icon = "jabber_na";
					break;
		default:
					icon = "jabber_offline";
					break;
	}
	
	return icon;
	
}

void JabberContact::slotRemoveThisUser()
{

	// unsubscribe
	mProtocol->removeUser(userID());
	
	// FIXME: should we destroy ourselves here? MSN does not do this...
	delete this;

}

void JabberContact::slotMoveThisUser()
{

	// assign new group to our contact
	if (!(mGroup = actionContactMove->currentText()))
		hasLocalGroup = false;
	else
		hasLocalGroup = true;
	
	// pass new group on to protocol backend
	mProtocol->moveUser(userID(), mGroup, displayName(), this);

}

void JabberContact::slotSendAuth()
{

}

void JabberContact::slotRerequestAuth()
{

}

void JabberContact::addToGroup(const QString &group)
{

}

void JabberContact::moveToGroup(const QString &from, const QString &to)
{

	if(to == "")
		hasLocalGroup = false;
	else
		hasLocalGroup = true;
		
	// pass new group on to protocol backend
	mProtocol->moveUser(userID(), to, displayName(), this);

}

void JabberContact::removeFromGroup(const QString &group)
{

	hasLocalGroup = false;
	
	// pass empty group to protocol backend
	mProtocol->moveUser(userID(), "", displayName(), this);

}

int JabberContact::importance() const
{
	int value;
	
	switch(mStatus)
	{
		case STATUS_ONLINE:
					value = 20;
					break;
		case STATUS_AWAY:
					value = 15;
					break;
		case STATUS_XA:
					value = 12;
					break;
		case STATUS_DND:
					value = 10;
					break;
		default:
					value = 0;
					break;
	}
	
	return value;
	
}

void JabberContact::slotChatThisUser()
{

	kdDebug() << "[JabberContact] Opening chat with user " << userID() << endl;
	
	msgManagerKCW()->readMessages();

}

void JabberContact::slotEmailUser()
{
	
	kdDebug() << "[JabberContact] Opening message with user " << userID() << endl;
	
	msgManagerKEW()->readMessages();
	msgManagerKEW()->slotSendEnabled(true);

}

void JabberContact::execute()
{

	KGlobal::config()->setGroup("Jabber");
	
	if (KGlobal::config()->readBoolEntry("EmailDefault", false))
		// user selected email window as preference
		slotEmailUser();
	else
		// user selected chat window as preference
		slotChatThisUser();

}

void JabberContact::slotNewMessage(const JabMessage &message)
{
	QString theirUserID = QString("%1@%2").arg(message.from.user(), 1).arg(message.from.host());

	// make sure we don't process other user's messages
	if (theirUserID != userID())
		return;


	// FIXME: is this necessary?
	KopeteContactPtrList contactList;
	contactList.append(mProtocol->myself());

	// create new message
	KopeteMessage jabberMessage(this, contactList, message.body, message.subject, KopeteMessage::Inbound);
	
	// depending on the incoming message type,
	// append it to the correct widget
	if (message.type == JABMESSAGE_CHAT)
		msgManagerKCW()->appendMessage(jabberMessage);
	else
	{
		msgManagerKEW()->appendMessage(jabberMessage);
		msgManagerKEW()->slotSendEnabled(false);
	}
}

void JabberContact::slotViewHistory()
{

	if (historyDialog == 0L)
	{
		historyDialog = new KopeteHistoryDialog(QString("jabber_logs/%1.log").arg(userID()), displayName(), true, 50, 0, "JabberHistoryDialog");
		connect(historyDialog, SIGNAL(closing()), this, SLOT(slotCloseHistoryDialog()));
	}
	
	historyDialog->show();
	historyDialog->raise();

}

void JabberContact::slotCloseHistoryDialog()
{
    
	delete historyDialog;
	historyDialog = 0L;

}

void JabberContact::slotSendMsgKEW(const KopeteMessage& message)
{
	JabMessage jabMessage;
	JabberContact *to = dynamic_cast<JabberContact *>(message.to().first());
	const JabberContact *from = dynamic_cast<const JabberContact *>(message.from());
	
	kdDebug() << "[JabberContact] slotSendMsg called: to " << to->userID() << ", from " << from->userID() << ", body: " << message.body() << "." << endl;
	
	// if a resource has been selected for this contact,
	// send to this special resource - otherwise,
	// just send to the server and let the server decide
	if (hasResource)
		jabMessage.to = QString("%1/%2").arg(to->userID(), 1).arg(activeResource->resource(), 2);
	else
		jabMessage.to = to->userID();

	jabMessage.from = from->userID();
	jabMessage.body = message.body();
	jabMessage.type = JABMESSAGE_NORM;
	jabMessage.subject = message.subject();
	
	// pass message on to protocol backend
	mProtocol->slotSendMsg(jabMessage);
	
	// append message to window
	msgManagerKEW()->appendMessage(message);

}

void JabberContact::slotSendMsgKCW(const KopeteMessage& message)
{
	JabMessage jabMessage;
	JabberContact *to = dynamic_cast<JabberContact *>(message.to().first());
	const JabberContact *from = dynamic_cast<const JabberContact *>(message.from());
	
	kdDebug() << "[JabberContact] slotSendMsg called: to " << to->userID() << ", from " << from->userID() << ", body: " << message.body() << "." << endl;
	
	// if a resource has been selected for this contact,
	// send to this special resource - otherwise,
	// just send to the server and let the server decide
	if (hasResource)
		jabMessage.to = QString("%1/%2").arg(to->userID(), 1).arg(activeResource->resource(), 2);
	else
		jabMessage.to = to->userID();

	jabMessage.from = from->userID();
	jabMessage.body = message.body();
	jabMessage.type = JABMESSAGE_CHAT;
	jabMessage.subject = "";
	
	// pass message on to protocol backend
	mProtocol->slotSendMsg(jabMessage);
	
	// append message to window
	msgManagerKCW()->appendMessage(message);

}

void JabberContact::slotResourceAvailable(const Jid &jid, const JabResource &resource)
{
	QString theirJID = QString("%1@%2").arg(jid.user(), 1).arg(jid.host(), 2);

	kdDebug() << "[JabberContact] New resource - they want " << theirJID << ", we're " << userID() << endl;
	
	// safety check: don't process resources of other users
	if (theirJID != userID())
		return;

	kdDebug() << "[JabberContact] Adding new resource '" << resource.name << "' for " << userID() << endl;

	/*
	 * if the new resource already exists, remove the current
	 * instance of it (Psi will emit resourceAvailable()
	 * whenever a certain resource changes status)
	 * removing the current instance will prevent double entries
	 * of the same resource.
	 * updated resource will be re-added below
	 * FIXME: this should be done using the QPtrList methods! (needs checking of pointer values)
	 */
	for (JabberResource *tmpResource = resources.first(); tmpResource; tmpResource = resources.next())
	{
		// FIXME: shouldn't this be resource instead of jid.resource?
		if (tmpResource->resource() == jid.resource())
		{
			kdDebug() << "[JabberContact] Resource " << tmpResource->resource() << " already added ... b0rk b0rk b0rk!" << endl;
			resources.remove();
		}
	}

	JabberResource *newResource = new JabberResource(resource.name , resource.priority, resource.timeStamp, resource.status, resource.statusString);
	resources.append(newResource);

	JabberResource *tmpBestResource = bestResource();

	kdDebug() << "[JabberContact] Best resource is now " << tmpBestResource->resource() << "." << endl;

	slotUpdateContact(tmpBestResource->resource(), tmpBestResource->status(), tmpBestResource->reason());

	// update to current best resource if resource override is not turned on
	if (hasResource == false)
	{
		activeResource = tmpBestResource;
	}

}

void JabberContact::slotResourceUnavailable(const Jid &jid)
{
	JabberResource *resource;
	QString theirJID = QString("%1@%2").arg(jid.user(), 1).arg(jid.host(), 2);
	
	kdDebug() << "[JabberContact] Removing resource - they want " << theirJID << ", we're " << userID() << endl;
	
	// safety check: don't process resources of other users
	if (theirJID != userID())
		return;

	kdDebug() << "[JabberContact] Removing resource '" << jid.resource() << "' for " << userID() << endl;

	for (resource = resources.first(); resource; resource = resources.next())
	{
		if (resource->resource() == jid.resource())
		{
			kdDebug() << "[JabberContact] Got a match in " << resource->resource() << ", removing." << endl;
			
			if (resources.remove())
				kdDebug() << "[JabberContact] Successfully removed, there are now " << resources.count() << " resources!" << endl;
			else
				kdDebug() << "[JabberContact] Ack! Couldn't remove the resource. Bugger!" << endl;

			break;
		}
	}
	
	JabberResource *newResource = bestResource();
	
	if (!newResource)
	{
		kdDebug() << "[JabberContact] No best resource! User is offline." << endl;
		slotUpdateContact("", STATUS_OFFLINE, "");
	}
	else
	{
		kdDebug() << "[JabberContact] Best resource is now " << newResource->resource() << "." << endl;
		slotUpdateContact(newResource->resource(), newResource->status(), newResource->reason());
	}
	
	// if we just deleted the current resource or there is no resource
	// override in effect, select a new resource
	if (hasResource == false || activeResource == resource)
	{
		/* No good sending to a deleted resource. */
		activeResource = bestResource();
		hasResource = false;
	}

}

void JabberContact::slotSelectResource()
{
	
	if (actionSelectResource->currentItem() == 0)
	{
		kdDebug() << "[JabberContact] Removing active resource, trusting bestResource()." << endl;
		
		hasResource = false;
		activeResource = bestResource();
	}
	else
	{
		QString selectedResource = actionSelectResource->currentText();

		kdDebug() << "[JabberContact] Moving to resource " << selectedResource << endl;

		hasResource = true;
		
		for (JabberResource *resource = resources.first(); resource; resource = resources.next())
		{
			if (resource->resource() == selectedResource)
			{
				kdDebug() << "[JabberContact] New active resource is " << resource->resource() << endl;

				activeResource = resource;

				break;
			}
		}
	}
	
}

void JabberContact::slotSnarfVCard()
{
	
	// just pass the request on to the protocol backend
	mProtocol->slotSnarfVCard(mUserID);
	
}

void JabberContact::slotGotVCard(JT_VCard *vCard)
{
	kdDebug() << "[JabberContact] Got vCard for user " << vCard->jid << ", displaying." << endl;
	
	dlgVCard = new dlgJabberVCard(kopeteapp->mainWindow(), "dlgJabberVCard", vCard);
	
	QObject::connect(dlgVCard, SIGNAL(updateNickname(const QString)), SLOT(slotUpdateNickname(const QString)));
	
	dlgVCard->show();
	dlgVCard->raise();

}

JabberResource *JabberContact::bestResource()
{
	JabberResource *resource, *tmpResource;
	
	// iterate through all available resources
	for (resource = tmpResource = resources.first(); tmpResource; tmpResource = resources.next())
	{
		kdDebug() << "[JabberContact] Processing resource " << tmpResource->resource() << endl;
		
		if (tmpResource->priority() > resource->priority())
		{
			kdDebug() << "[JabberContact] Got better resource " << tmpResource->resource() << " through better priority." << endl;
			resource = tmpResource;
		}
		else
		{
			if (tmpResource->priority() == resource->priority())
			{
				if (tmpResource->timestamp() > resource->timestamp())
				{
					kdDebug() << "[JabberContact] Got better resource " << tmpResource->resource() << " through newer timestamp." << endl;
					resource = tmpResource;
				}
				else
				{
					kdDebug() << "[JabberContact] Discarding resource " << tmpResource->resource() << " with older timestamp." << endl;
				}
			}
			else
			{
				kdDebug() << "[JabberContact] Discarding resource " << tmpResource->resource() << " with worse priority." << endl;
			}
		}
	}
	
	return resource;

}

void JabberContact::slotRemoveFromGroup()
{

	// move to and empty group
	mProtocol->moveUser(userID(), mGroup = QString(""), displayName(), this);
	
	// set flag accordingly
	hasLocalGroup = false;

}

QString JabberContact::id() const
{

	return mUserID;

}

QString JabberContact::data() const
{

	return mUserID;

}

void JabberContact::slotUpdateNickname(const QString newNickname)
{
	kdDebug() << "JabberContact::slotUpdateNickname( " << newNickname << " )" << endl;
	
	if ( newNickname == displayName() )
		// new nickname matches old nickname, don't change
		return;

	if (newNickname == "")
	{
		KMessageBox::sorry(0L, i18n("There should be at least one character for the nickname!"), i18n("No nickname"));
		return;
	}
	
	// send rename to protocol backend
	mProtocol->renameContact(mUserID, newNickname, mGroup);
	
	// update display
	setDisplayName( newNickname );

}

#include "jabbercontact.moc"

// vim: noet ts=4 sts=4 sw=4:
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

