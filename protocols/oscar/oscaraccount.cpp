/*
    oscaraccount.cpp  -  Oscar Account Class

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Copyright (c) 2002 by Chris TenHarmsel <tenharmsel@staticmethod.net>
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

#include "oscaraccount.h"
#include "aim.h"
#include "ssidata.h"

#include "kopeteprotocol.h"
#include "kopeteaway.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopeteawaydialog.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include "kopetecontactlist.h"

#include <assert.h>

#include <qapplication.h>
#include <qregexp.h>
#include <qstylesheet.h>
#include <qtimer.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

class OscarAccountPrivate
{
public:
	/**
	 * Server-side contacts that do not have KopeteContacts yet for the reason that
	 * their group has not yet been sent from the server
	 * We queue them so that we can create KopeteContacts for them later
	 */
	QPtrList<SSI> groupQueue;

	/*
	 * Our OSCAR socket object
	 */
	OscarSocket *engine;

	/*
	 * Random new group/contact number for the engine
	 */
	//int randomNewGroupNum;
	//int randomNewBuddyNum;

	/*
	 * This flag is used internally to keep track
	 * of if we're idle or not
	 */
	bool isIdle;

	/*
	 * Last idle time in seconds we sent to the server
	 */
	int lastIdleValue;

	/*
	 * anti SPAM feature :)
	 */
	bool ignoreUnknownContacts;

	/*
	 * This is our idle timer, it is used internally
	 * to represent idle times and report them to
	 * the server
	 */
	QTimer *idleTimer;

	//QString awayMessage;

	bool passwordWrong;
};

OscarAccount::OscarAccount(KopeteProtocol *parent, const QString &accountID, const char *name, bool isICQ)
: KopeteAccount( parent, accountID, name )
{
	kdDebug(14150) << k_funcinfo << " accountID='" << accountID <<
		"', isICQ=" << isICQ << endl;

	d = new OscarAccountPrivate;

	d->engine = 0L;
	// Set our random new numbers
	//d->randomNewBuddyNum = 0;
	//d->randomNewGroupNum = 0;
	d->ignoreUnknownContacts = false;
	d->isIdle = false;
	d->lastIdleValue = 0;
	//d->awayMessage = "";
	d->passwordWrong = false;

	initEngine(isICQ); // Initialize the backend

	// Contact list signals for group management events
	QObject::connect(
		KopeteContactList::contactList(), SIGNAL(groupRenamed(KopeteGroup *, const QString &)),
		this, SLOT(slotKopeteGroupRenamed(KopeteGroup *, const QString &)));

	QObject::connect(
		KopeteContactList::contactList(), SIGNAL(groupRemoved(KopeteGroup *)),
		this, SLOT(slotKopeteGroupRemoved(KopeteGroup *)));

	// own status changed
	QObject::connect(
		engine(), SIGNAL(statusChanged(const unsigned int)),
		this, SLOT(slotOurStatusChanged(const unsigned int)));

	// Protocol error
	QObject::connect(
		engine(), SIGNAL(protocolError(QString, int, bool)),
		this, SLOT(slotError(QString, int, bool)));

	QObject::connect(
		engine(), SIGNAL(receivedMessage(const QString &, OscarMessage &)),
		this, SLOT(slotReceivedMessage(const QString &, OscarMessage &)));

	QObject::connect(
		engine(), SIGNAL(receivedAwayMessage(const QString &, const QString &)),
		this, SLOT(slotReceivedAwayMessage(const QString &, const QString &)));

	// Got Config (Buddy List)
	QObject::connect(
		engine(), SIGNAL(gotConfig()),
		this, SLOT(slotGotServerBuddyList()));

	// Got direct IM request
#if 0
	QObject::connect(
		engine(), SIGNAL(gotDirectIMRequest(QString)),
		this, SLOT(slotGotDirectIMRequest(QString)));
#endif

	d->idleTimer = new QTimer(this, "OscarIdleTimer");
	QObject::connect(
		d->idleTimer, SIGNAL(timeout()),
		this, SLOT(slotIdleTimeout()));

	QObject::connect( d->engine, SIGNAL( loggedIn() ), this, SLOT( slotLoggedIn() ) );
}

OscarAccount::~OscarAccount()
{
	//kdDebug(14150) << k_funcinfo << "'" << accountId() << "'" << endl;

	OscarAccount::disconnect(KopeteAccount::Manual);

	// Delete the backend
	if (d->engine)
	{
		//kdDebug(14150) << k_funcinfo << "'" << accountId() << "'; delayed deleting of d->engine" << endl;
		d->engine->deleteLater();
	}

	delete d;
}

OscarSocket* OscarAccount::engine() const
{
	assert(d->engine);
	return d->engine;
}

void OscarAccount::disconnect()
{
	OscarAccount::disconnect(KopeteAccount::Manual);
}

void OscarAccount::disconnect(DisconnectReason reason)
{
	kdDebug(14150) << k_funcinfo << "accountId='" << accountId() << "'" << endl;
	d->engine->doLogoff();
	KopeteAccount::disconnect(reason);
}

bool OscarAccount::passwordWasWrong()
{
	return d->passwordWrong;
}

void OscarAccount::initEngine(bool icq)
{
	kdDebug(14150) << k_funcinfo << "for account '" << accountId() << "'" << endl;

	QByteArray cook;
	cook.duplicate("01234567",8);
	d->engine = new OscarSocket(pluginData(protocol(),"Server"), cook,
		this, this, "engine", icq);
}

void OscarAccount::slotGoOffline()
{
	OscarAccount::disconnect(KopeteAccount::Manual);
}

void OscarAccount::slotError(QString errmsg, int errorCode, bool isFatal)
{
	kdDebug(14150) << k_funcinfo << "accountId='" << accountId() <<
		"' errmsg=" << errmsg <<
		", errorCode=" << errorCode << "." << endl;

	if (isFatal)
		OscarAccount::disconnect(KopeteAccount::Manual);

	// suppress error dialog for password-was-wrong error, since we're about
	// to pop up a password dialog saying the same thing when we try to reconenct
	if (errorCode == 4 || errorCode == 5)
	{
		d->passwordWrong = true;
		QTimer::singleShot(0, this, SLOT(connect()));
	}
	else
	{
		QString caption = engine()->isICQ() ?
			i18n("Connection Lost - ICQ Plugin") :
			i18n("Connection Lost - AIM Plugin");

		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(),
			KMessageBox::Error, errmsg, caption, KMessageBox::Notify);
	}
}

void OscarAccount::slotReceivedMessage(const QString &sender, OscarMessage &incomingMessage)
{
	kdDebug(14150) << k_funcinfo << "account='" << accountId() <<
		"', type=" << static_cast<int>(incomingMessage.type()) <<
		", sender='" << sender << "'" << endl;

	OscarContact *contact = static_cast<OscarContact*>(contacts()[tocNormalize(sender)]);
	QString text = incomingMessage.text();

	if(!contact && !d->ignoreUnknownContacts)
	{
		//basically, all this does is add the person to your kopete local list
		//Temporary contacts will be handled by their respective subclasses
		//(AIMContact and ICQContact)

		kdDebug(14150) << k_funcinfo <<
			"Message from contact that is not on our contactlist, sender='" <<
			sender << "'" << endl;

		if (addContact(tocNormalize(sender), sender, 0L, KopeteAccount::DontChangeKABC, QString::null, true))
			contact = static_cast<OscarContact*>(contacts()[tocNormalize(sender)]);
		else
			return; // adding contact failed for whatever reason!
	}

	if (contact)
	{
		switch(incomingMessage.type())
		{
			case OscarMessage::Away:
				text = i18n("<b>[Away Message:]</b> %1").arg(text);
				break;

			case OscarMessage::URL:
				text.replace("þ", "<br />");
				text=i18n("<b>[URL Message:]</b> %1").arg(text);
				break;

			case OscarMessage::SMS:
				text=i18n("<b>[SMS Message:]</b> %1").arg(text);
				break;

			case OscarMessage::EMail:
				text=i18n("<b>[Email Message:]</b> %1").arg(text);
				break;

			case OscarMessage::WebPanel:
				text.replace(QString::fromLatin1("þþþ"), QString::fromLatin1("<br />"));
				text.replace(QString::fromLatin1("þ3þ"), QString::fromLatin1("<br />"));
				text=i18n("<b>[WebPanel Message:]</b> %1").arg(text);
				break;

			case OscarMessage::Normal:
				break;

			case OscarMessage::GrantedAuth:
				text=i18n("<b>[Granted authentication:]</b> %1").arg(text);
				break;

			case OscarMessage::DeclinedAuth:
				text=i18n("<b>[Declined authentication:]</b> %1").arg(text);
				break;
		}

		KopeteContactPtrList tmpList;
		tmpList.append(myself());

		KopeteMessage kmsg(
			incomingMessage.timestamp, contact, tmpList, text, KopeteMessage::Inbound,
			KopeteMessage::RichText);

		kmsg.setFg(incomingMessage.fgColor);
		kmsg.setBg(incomingMessage.bgColor);

		contact->receivedIM(kmsg);
	}
}

void OscarAccount::slotReceivedAwayMessage(const QString &sender, const QString &message)
{
	/*kdDebug(14150) << k_funcinfo << "account='" << accountId() <<
		", sender='" << sender << "'" << endl;*/

	OscarContact *contact = static_cast<OscarContact*>(contacts()[tocNormalize(sender)]);
	if(!contact)
		return;

	/*kdDebug(14150) << k_funcinfo <<
		"Incoming away-message is: " << message << endl;*/

	QString filteredMessage = message;
	filteredMessage.replace(
		QRegExp(QString::fromLatin1("<[hH][tT][mM][lL].*>(.*)</[hH][tT][mM][lL]>")),
		QString::fromLatin1("\\1"));
	filteredMessage.replace(
		QRegExp(QString::fromLatin1("<[bB][oO][dD][yY].*>(.*)</[bB][oO][dD][yY]>")),
		QString::fromLatin1("\\1") );
	filteredMessage.replace(
		QRegExp(QString::fromLatin1("<[fF][oO][nN][tT].*>(.*)</[fF][oO][nN][tT]>")),
		QString::fromLatin1("\\1") );

	/*kdDebug(14150) << k_funcinfo <<
		"Filtered Away-message is: " << filteredMessage << endl;*/
	contact->setAwayMessage(filteredMessage);
}

// Called when a group is added by adding a contact
void OscarAccount::slotGroupAdded(KopeteGroup *group)
{
	if ( !isConnected() )
		return;

	//Don't add top level or temp groups to contact lists
	if ( group->type() == KopeteGroup::TopLevel || group->type() == KopeteGroup::Temporary )
		return;

	kdDebug(14150) << k_funcinfo <<
		"called, groupname='" << group->displayName() << "'" << endl;

	QString groupName = group->displayName();
	if(groupName.isEmpty())
	{
		kdDebug(14150) << k_funcinfo <<
			"I'm not adding groups with no names!" << endl;
		return;
	}

	/* If group found in SSI, don't add. Otherwise, add group to SSI */
	if ( !engine()->ssiData().findGroup( group->displayName() ) )
		engine()->sendAddGroup( group->displayName() );

}

void OscarAccount::slotKopeteGroupRenamed(KopeteGroup *group, const QString &oldName)
{
	if ( !isConnected() )
		return;

	kdDebug(14150) << k_funcinfo << "Sending 'group rename' to server" << endl;
	engine()->sendChangeGroupName(oldName, group->displayName());
}


void OscarAccount::slotKopeteGroupRemoved(KopeteGroup *group)
{
	if (!isConnected())
		return;

	// This method should be called after the contacts have been removed
	// We should then be able to remove the group from the server
	kdDebug(14150) << k_funcinfo <<
		"Sending 'delete group' to server, group='" <<
		group->displayName() << "'" << endl;

	QString groupName = group->displayName();
	if (groupName.isEmpty())
	{
		kdDebug(14150) << k_funcinfo << "Removing a group with no name!" << endl;
		return;
	}

	/* Check to make sure group is found in SSI, then delete */
	if ( engine()->ssiData().findGroup( groupName ) )
		engine()->sendDelGroup( groupName );
}


void OscarAccount::slotGotServerBuddyList()
{
	kdDebug(14150) << k_funcinfo << "account='" << accountId() << "'" << endl;

	//If we get mysterious results (or crashes) here, it's because the SSIData object
	//was mysteriously destroyed and since engine()->ssiData() returns a reference
	//we'll need to start saving the result of engine()->ssiData() so that we
	//make sure it lives long enough.
	QPtrListIterator<SSI> git(engine()->ssiData());
	for ( ; git.current(); ++git )
	{
		if (git.current()->type == ROSTER_GROUP && !git.current()->name.isEmpty())
		{ //active contact on SSI
			kdDebug(14150) << k_funcinfo << "Adding group '" <<
				git.current()->name << "' to contact list" << endl;
			addGroup( git.current()->name );
		}
	}

	//groups are added. Add the contacts
	QPtrListIterator<SSI> bit(engine()->ssiData());
	QString groupName;
	OscarContact* contact = 0;

	for (; bit.current(); ++bit)
	{
		switch (bit.current()->type)
		{
			case ROSTER_CONTACT:
			{ //active contact on SSI
				SSI* ssiGroup = engine()->ssiData().findGroup(bit.current()->gid);
				if (ssiGroup==0)
				{
					kdDebug(14150) << k_funcinfo <<
						"ssiGroup invalid for some reason. Using group name 'Buddies'" << endl;
					groupName = i18n("Buddies");
				}
				else
				{
					/*kdDebug(14150) << k_funcinfo <<
						"ssiGroup is valid using group name = '" << ssiGroup->name << "'" << endl;
					*/
					groupName = ssiGroup->name;
				}

				contact = static_cast<OscarContact*> (contacts()[tocNormalize(bit.current()->name)]);
				if (contact == 0)
				{
					kdDebug(14150) << k_funcinfo "Adding SSI contact '" <<
						bit.current()->name << "' to contact list" << endl;
					addContact(tocNormalize(bit.current()->name),
						bit.current()->name, 0L,
						DontChangeKABC, groupName, false);
					//make sure we set the contact to be on the SSI
					//contact->setServerSide( true );
				}
				break;
			}

			case ROSTER_VISIBLE: // a contact on the allow(AIM) / visible(ICQ) list
			{
				contact = static_cast<OscarContact*> (contacts()[tocNormalize(bit.current()->name)]);
				if (contact != 0)
				{
					kdDebug(14150) << k_funcinfo <<
						"Setting VISIBLE TO flag on contact '" << contact->displayName() << "'" << endl;
					contact->setVisibleTo(true);
				}
				break;
			}

			case ROSTER_INVISIBLE:// a contact on the block(AIM) / invisible(ICQ) list
			{
				contact = static_cast<OscarContact*> (contacts()[tocNormalize(bit.current()->name)]);
				if (contact != 0)
				{
					kdDebug(14150) << k_funcinfo <<
						"Setting INVISIBLE flag on contact '" <<
						contact->displayName() << "'" << endl;
					contact->setInvisibleTo(true);
				}
				break;
			}

			case ROSTER_IGNORE: // a contact on the ignore list (ICQ only?)
			{
				contact = static_cast<OscarContact*> (contacts()[tocNormalize(bit.current()->name)]);
				if (contact != 0)
				{
					kdDebug(14150) << k_funcinfo <<
						"Setting IGNORE flag on contact '" <<
						contact->displayName() << "'" << endl;
					contact->setIgnore(true);
				}
				break;
			}
		}
	}

	//Get a list of contacts, if they're not on the SSI, move them to a new
	//metacontact, and then move that new metacontact to the temp group
	//so they're gone on kopete exit
	/*
	QDictIterator<KopeteContact> it(contacts());
	for (; it.current(); ++it)
	{
		OscarContact* c = static_cast<OscarContact*>((*it));
		//get the metacontact so we can remove it if it's empty
		KopeteMetaContact *oldContact = c->metaContact();
		if ( c->metaContact() && !c->serverSide())
		{
			kdDebug(14150) << "Moving '" << c->displayName() << "' to temporary" << endl;
			c->metaContact()->removeContact(c);
			KopeteMetaContact* kmc = new KopeteMetaContact;
			c->setMetaContact(kmc);
			kmc->setTemporary(true);
			KopeteContactList::contactList()->addMetaContact( kmc );

			if (oldContact->contacts().isEmpty())
			{
				KopeteContactList::contactList()->removeMetaContact(oldContact);
				delete oldContact;
			}
		}
	}*/

	QObject::connect(KopeteContactList::contactList(), SIGNAL(groupAdded(KopeteGroup *)),
		this, SLOT(slotGroupAdded(KopeteGroup *)));
}


void OscarAccount::slotLoggedIn()
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;

	d->passwordWrong = false;

	// Only call sync if we received a list on connect, does not happen on @mac AIM-accounts
#if 0
	if ( engine()->ssiData().isEmpty() )
	{
		QTimer::singleShot(2000, this, SLOT(slotDelayedListSync()));
	}
#endif

	d->idleTimer->start(10 * 1000);
}


#if 0
void OscarAccount::slotDelayedListSync()
{
	kdDebug(14150) << k_funcinfo << "Called ==============================================" << endl;

	syncLocalWithServerBuddyList ();
}

//
// Since the icq_new transition, I have lots of contacts, that are not in the serverside
// contact list. So we compare the two lists and add all local contacts that are not
// in the server side list.
//

void OscarAccount::syncLocalWithServerBuddyList()
{
	kdDebug(14150) << k_funcinfo << "Called but DISABLED =========================================" << endl;
	//FIXME: Does not work [mETz]
	const QDict<KopeteContact>& contactList = contacts();
	QDictIterator<KopeteContact> it( contactList );

	for(; it.current(); ++it)
	{
		QString contactId = static_cast<OscarContact*>( it.current() )->contactName();
		QString displayName = static_cast<OscarContact*>( it.current() )->displayName();

		AIMBuddy *buddy = d->loginContactlist->findBuddy( contactId );

		if(!buddy && it.current() != myself())
		{
			kdDebug(14150) << "###### Serverside list doesn't contain local buddy: " << contactId << endl;
			kdDebug(14150) << "-> creating server side contact for it." << endl;
			// Add the buddy to the server's list
			const KopeteGroupList& groups = it.current()->metaContact()->groups();

			AIMGroup *group = findOrCreateGroup( groups.isEmpty() ? QString::null : groups.getFirst()->displayName() );

			// Create a new internal buddy for this contact
			AIMBuddy *newBuddy =
				new AIMBuddy(d->randomNewBuddyNum++, group->ID(), tocNormalize(contactId));

			// Check if it has an alias
			if((displayName != QString::null) && (displayName != contactId))
				newBuddy->setAlias(displayName);

			// Add the buddy to the internal buddy list
			addBuddy( newBuddy );

			// Add the buddy to the server's list, with the group,
			// need to normalize the contact name
			engine()->sendAddBuddy(tocNormalize(contactId), group->name());
		}
	}
}
#endif

#if 0
void OscarAccount::slotGotDirectIMRequest(QString sn)
{
	//FIXME: This is a stupid question, ICQ always accepts
	// these as long as you allow direct connections in general

	QString title = i18n("Direct IM session request");
	QString message =
	i18n("%1 has requested a direct IM session with you. " \
		"Direct IM sessions allow the remote user to see your IP " \
		"address, which can lead to security problems if you don't " \
		"trust him/her. Do you want to establish a direct connection " \
		"to %2?")
#if QT_VERSION < 0x030200
			.arg(sn).arg(sn);
#else
			.arg(sn,sn);
#endif

	int result = KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(),
		message, title);

	if (result == KMessageBox::Yes)
		engine()->sendDirectIMAccept(sn);
	else if (result == KMessageBox::No)
		engine()->sendDirectIMDeny(sn);
}
#endif


void OscarAccount::slotIdleTimeout()
{
	//kdDebug(14150) << k_funcinfo << "called" << endl;
	int idletime = KopeteAway::getInstance()->idleTime();
	// not doing anything for more than 5 mins and still not idle
	if(idletime >= 5*60)
	{
		if (idletime >= d->lastIdleValue + 60)
		{
			/*kdDebug(14150) << k_funcinfo <<
				"sending idle time to server, idletime=" << idletime << endl;*/
			engine()->sendIdleTime(idletime);
			d->lastIdleValue = idletime;

			// Set the idle flag
			d->isIdle = true;
		}
	}
	else
	{
		if (d->isIdle)
		{ // If we _are_ idle, change it
			/*kdDebug(14150) << k_funcinfo <<
				"system change to ACTIVE, setting idle time with server to 0" << endl;*/
			engine()->sendIdleTime(0);
			d->isIdle = false;
			d->lastIdleValue = 0;
		}
	}
}


/*int OscarAccount::randomNewBuddyNum()
{
	return d->randomNewBuddyNum++;
}


int OscarAccount::randomNewGroupNum()
{
	return d->randomNewGroupNum++;
}*/


void OscarAccount::setServerAddress(const QString &server)
{
	kdDebug(14150) << k_funcinfo << "Called, server=" << server << endl;
	setPluginData(protocol(), "Server", server);
}

void OscarAccount::setServerPort(int port)
{
	kdDebug(14150) << k_funcinfo << "Called, port=" << port << endl;
	if (port > 0)// Do a little error checking on it
		setPluginData(protocol(), "Port", QString::number(port));
}

void OscarAccount::addGroup( const QString& groupName )
{
	KopeteGroup* group = KopeteContactList::contactList()->getGroup( groupName );
	Q_UNUSED( group );
}


#if 0
void OscarAccount::addOldContact( SSI* ssiItem, KopeteMetaContact* meta )
{
	bool temporary = false;
	SSI* group = engine()->ssiData().findGroup( ssiItem->gid );

	if ( !group )
	{	// group not yet found
		kdDebug(14150) << k_funcinfo << "Adding '" << group->name << "' to groupQueue "
			<< "to add later when group is received" << endl;
		d->groupQueue.append( ssiItem );
		return;
	}

	KopeteMetaContact* m = KopeteContactList::contactList()->findContact(
		protocol()->pluginId(), accountId(), ssiItem->name);

	if ( m && m->isTemporary() )
	{
		m->setTemporary( false );
		//m->moveToGroup( KopeteGroup::topLevel(), KopeteContactList::contactList()->getGroup( group->name ) );
	}
	else
	{
		//New Contact
		kdDebug(14150) << k_funcinfo << "Adding old contact '" <<
			ssiItem->name << "'" << endl;

		if ( meta )
			m = meta;
		else
		{
			m = new KopeteMetaContact();
			m->addToGroup( KopeteContactList::contactList()->getGroup( group->name ) );
		}

		if ( temporary )
			m->setTemporary( true );

		createNewContact( tocNormalize(ssiItem->name), ssiItem->name, m);

		if ( !meta )
			KopeteContactList::contactList()->addMetaContact( m );
	}
}
#endif


bool OscarAccount::addContactToMetaContact(const QString &contactId,
	const QString &displayName, KopeteMetaContact *parentContact)
{
	/*
	 * This method is called in three situations.
	 * The first one is when the user, through the GUI
	 * adds a buddy and it happens to be from this account.
	 * In this situation, we need to create a new group if
	 * necessary and add the contact to the server
	 *
	 * The second situation is where we are loading a server-
	 * side contact list through the method addServerContact
	 * which calls addContact, which in turn calls this method.
	 * To cope with this situation we need to first check if
	 * the contact already exists in the internal list.
	 *
	 *
	 * The third situation is when somebody new messages you
	 */

	/* We're not even online or connecting
	 * (when getting server contacts), so don't bother
	 */

	if ((!myself()->isOnline()) &&
		(myself()->onlineStatus().status() != KopeteOnlineStatus::Connecting))
	{
		kdDebug(14150) << k_funcinfo << "Can't add contact, we are offline!" << endl;
		return false;
	}

	// Next check our internal list to see if we have this buddy
	SSI* ssiItem = engine()->ssiData().findContact( contactId );
	if ( ssiItem )
	{
		kdDebug(14150) << k_funcinfo <<
			"Found contact on internal list. Making new OscarContact" << endl;
		OscarContact* newContact = createNewContact(contactId, displayName, parentContact, true);
		if ( newContact )
		{
			newContact->setStatus(OSCAR_OFFLINE);
			return true;
		}
		else
			return false;
	}
	else // Not on SSI. Must be new contact
	{
		kdDebug(14150) << k_funcinfo << "New contact '" << contactId << "' not in SSI."
			<< " Creating new contact" << endl;

		if ( !parentContact->isTemporary() )
		{
			kdDebug(14150) << k_funcinfo << "Adding contact to the server side list" << endl;

			QString groupName;
			KopeteGroupList kopeteGroups = parentContact->groups(); //get the group list

			if (kopeteGroups.isEmpty() ||
				kopeteGroups.first()->type() == KopeteGroup::TopLevel)
			{
				kdDebug(14150) << k_funcinfo << "Contact with NO group. "
					<< "Adding to group 'Buddies'" << endl;
				groupName = i18n("Buddies");
			}
			else
			{
				//OSCAR doesn't support multiple groups for a contact. Add to the
				//first one
				groupName = kopeteGroups.first()->displayName();

				kdDebug(14150) << k_funcinfo << "Contact with group." <<
					" No. of groups = " << kopeteGroups.count() <<
					" Name of first group = " << groupName << endl;
			}

			if(groupName.isEmpty())
			{ // emergency exit, should never occur
				kdWarning(14150) << k_funcinfo <<
					"Could not add Contact because no " <<
					"groupname was given" << endl;
				return false;
			}

			// Add the buddy to the server's list, with the group,
			// need to normalize the contact name
			// NOTE: sendAddBuddy autocreates a group named groupName if it's not on SSI
			engine()->sendAddBuddy(tocNormalize(contactId), groupName, false);

			// Increase these counters, I'm not sure what this does
			//d->randomNewGroupNum++;
			//d->randomNewBuddyNum++;

			// Create the actual contact, which adds it to the metacontact
			return(createNewContact(contactId, displayName, parentContact, true));
		}
		else
		{
			kdDebug(14150) << "Temporary new contact, only adding him to local list" << endl;
			// This is a temporary contact, so don't add it to the server list
			// Create the contact, which adds it to the parent contact
			if(!createNewContact(contactId, displayName, parentContact))
				return false;

			// Get user status through BLM if contact is temporary (only works on ICQ)
			if (engine()->isICQ())
				engine()->sendAddBuddylist(tocNormalize(contactId));

			// Set it's initial status
			// This requests the buddy's info from the server
			// I'm not sure what it does if they're offline, but there
			// is code in oscarcontact to handle the signal from
			// the engine that this causes
			kdDebug(14150) << k_funcinfo <<
				"Requesting user info for '" << contactId << "'" << endl;
			engine()->sendUserLocationInfoRequest(tocNormalize(contactId), AIM_LOCINFO_SHORTINFO);

			return true;
		}
	} // END not ssiItem
}


void OscarAccount::slotOurStatusChanged(const unsigned int newStatus)
{
	kdDebug(14150) << k_funcinfo << "Called; newStatus=" << newStatus << endl;
	static_cast<OscarContact *>(myself())->setStatus(newStatus);

	if(newStatus == OSCAR_OFFLINE)
		d->idleTimer->stop();
}

void OscarAccount::setAwayMessage(const QString &msg)
{
	static_cast<OscarContact *>(myself())->setAwayMessage(msg);
}

const QString OscarAccount::awayMessage()
{
	return (static_cast<OscarContact *>(myself())->awayMessage());
}

bool OscarAccount::ignoreUnknownContacts() const
{
	return d->ignoreUnknownContacts;
}

void OscarAccount::setIgnoreUnknownContacts( bool b )
{
	d->ignoreUnknownContacts = b;
}

#include "oscaraccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

