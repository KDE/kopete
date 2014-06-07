 /*
  * jabbercontact.cpp  -  Regular Kopete Jabber protocol contact
  *
  * Copyright (c) 2002-2004 by Till Gerken <till@tantalo.net>
  * Copyright (c) 2006     by Olivier Goffart <ogoffart at kde.org>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#include "jabbercontact.h"

#include "xmpp_tasks.h"
#include "im.h"
//#include "td.h"
#include "tasks/jt_getlastactivity.h"

#include <qtimer.h>
#include <qdatetime.h>
#include <QTextDocument>
#include <qimage.h>
#include <qregexp.h>
#include <qbuffer.h>
#include <QList>

#include <KActionCollection>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kaction.h>
#include <kactionmenu.h>
#include <kicon.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kinputdialog.h>
#include <kopeteview.h>

#include "kopetecontactlist.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include "kopetechatsessionmanager.h"
#include "kopeteaccountmanager.h"
#include "kopetemetacontact.h"
#include "kopetepluginmanager.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberclient.h"
#include "jabberchatsession.h"
#include "jabberresource.h"
#include "jabberresourcepool.h"
#include "jabberfiletransfer.h"
#include "jabbertransport.h"
#include "dlgjabbervcard.h"

#ifdef JINGLE_SUPPORT
#include "jinglecallsmanager.h"
#endif

/**
 * JabberContact constructor
 */
JabberContact::JabberContact (const XMPP::RosterItem &rosterItem, Kopete::Account *_account, Kopete::MetaContact * mc, const QString &legacyId)
	: JabberBaseContact ( rosterItem, _account, mc, legacyId)  , mDiscoDone(false), m_syncTimer(0L)
{
	///*account()->client()->clientStream()->*/XMPP::setDebug((Debug*) new TD());
	kDebug(JABBER_DEBUG_GLOBAL) << contactId() << "  is created  - " << this;
	// this contact is able to transfer files
	setFileCapable ( true );
	

	/*
	 * Catch when we're going online for the first time to
	 * update our properties from a vCard. (properties are
	 * not available during startup, so we need to read
	 * them later - this also serves as a random update
	 * feature)
	 * Note: The only time account->myself() could be a
	 * NULL pointer is if this contact here is the myself()
	 * instance itself. Since in that case we wouldn't
	 * get updates at all, we need to treat that as a
	 * special case.
	 */

	mVCardUpdateInProgress = false;

	if ( !account()->myself () )
	{
		// JabberContact (this) is the myself instance
		connect ( this,
				  SIGNAL (onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
				  this, SLOT (slotCheckVCard()) );
	}
	else
	{
		// JabberContact (this) is a regular contact and account()->myself () is the myself instance
		connect ( account()->myself (),
				  SIGNAL (onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
				  this, SLOT (slotCheckVCard()) );

		connect ( account()->myself (),
				  SIGNAL (onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
				  this, SLOT (slotCheckLastActivity(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)) );

		/*
		 * Trigger update once if we're already connected for contacts
		 * that are being added while we are online.
		 */
		if ( account()->myself()->onlineStatus().isDefinitelyOnline() )
		{
			mVCardUpdateInProgress = true;
			QTimer::singleShot ( 1000, this, SLOT (slotGetTimedVCard()) );
		}
	}

	mRequestOfflineEvent = false;
	mRequestDisplayedEvent = false;
	mRequestDeliveredEvent = false;
	mRequestComposingEvent = false;

	mRequestReceiptDelivery = false;
}

JabberContact::~JabberContact()
{
	kDebug(JABBER_DEBUG_GLOBAL) << contactId() << "  is destroyed  - " << this;
}

QList<KAction*> *JabberContact::customContextMenuActions ()
{

	QList<KAction*> *actions = new QList<KAction*>();

	KActionMenu *actionAuthorization = new KActionMenu ( KIcon("network-connect"), i18n ("Authorization"), this);

	KAction *resendAuthAction, *requestAuthAction, *removeAuthAction;
	
	resendAuthAction = new KAction( this );
	resendAuthAction->setIcon( (KIcon("mail-forward") ) );
	resendAuthAction->setText( i18n ("(Re)send Authorization To") );
	resendAuthAction->setEnabled( mRosterItem.subscription().type() == XMPP::Subscription::To || mRosterItem.subscription().type() == XMPP::Subscription::None );
	connect(resendAuthAction, SIGNAL(triggered(bool)), SLOT(slotSendAuth()));
	actionAuthorization->addAction(resendAuthAction);

	requestAuthAction = new KAction( this );
	requestAuthAction->setIcon( (KIcon("mail-reply-sender") ) );
	requestAuthAction->setText( i18n ("(Re)request Authorization From") );
	requestAuthAction->setEnabled( mRosterItem.subscription().type() == XMPP::Subscription::From || mRosterItem.subscription().type() == XMPP::Subscription::None );
	connect(requestAuthAction, SIGNAL(triggered(bool)), SLOT(slotRequestAuth()));
	actionAuthorization->addAction(requestAuthAction);
	
	removeAuthAction = new KAction( this );
	removeAuthAction->setIcon( (KIcon("edit-delete") ) );
	removeAuthAction->setText( i18n ("Remove Authorization From") );
	removeAuthAction->setEnabled( mRosterItem.subscription().type() == XMPP::Subscription::Both || mRosterItem.subscription().type() == XMPP::Subscription::From );
	connect(removeAuthAction, SIGNAL(triggered(bool)), SLOT(slotRemoveAuth()));
	actionAuthorization->addAction(removeAuthAction);

#ifdef LIBJINGLE_SUPPORT

	if ( account()->enabledLibjingle() ) {

		KAction *libjingleCallAction;
		libjingleCallAction = new KAction( this );
		libjingleCallAction->setIcon( (KIcon("voicecall") ) );
		libjingleCallAction->setText( i18n ("Call contact") );
		libjingleCallAction->setEnabled( account()->supportLibjingle(contactId()) );
		connect(libjingleCallAction, SIGNAL(triggered(bool)), SLOT(makeLibjingleCallAction()));
		actions->append(libjingleCallAction);

	}

#endif

	KActionMenu *actionSetAvailability = new KActionMenu ( KIcon("user-identity", 0, QStringList() << QString() << "user-online"), i18n ("Set Availability"), this );

#define KACTION(status, text, name, slot) \
	{ KAction *tmp = new KAction(this); \
	tmp->setIcon( KIcon(QIcon((status).iconFor(this)))); \
	tmp->setText( text ); \
	connect(tmp, SIGNAL(triggered(bool)), (slot));\
	actionSetAvailability->addAction(tmp); }

	KACTION( protocol()->JabberKOSOnline, i18n ("Online"),         "actionOnline", SLOT(slotStatusOnline()) );
	KACTION( protocol()->JabberKOSChatty, i18n ("Free to Chat"),   "actionChatty", SLOT(slotStatusChatty()) );
	KACTION( protocol()->JabberKOSAway,   i18n ("Away"),           "actionAway",   SLOT(slotStatusAway()) );
	KACTION( protocol()->JabberKOSXA,     i18n ("Extended Away"),  "actionXA",     SLOT(slotStatusXA()) );
	KACTION( protocol()->JabberKOSDND,    i18n ("Do Not Disturb"), "actionDND",    SLOT(slotStatusDND()) );
	KACTION( protocol()->JabberKOSInvisible, i18n ("Invisible"),   "actionInvisible", SLOT(slotStatusInvisible()) );

#undef KACTION

	KActionMenu *actionSelectResource = new KActionMenu ( i18n ("Select Resource"), this );

	// if the contact is online, display the resources we have for it,
	// otherwise disable the menu
	if (onlineStatus ().status () == Kopete::OnlineStatus::Offline)
	{
		actionSelectResource->setEnabled ( false );
	}
	else
	{
		QStringList items;
		XMPP::ResourceList availableResources;

		int activeItem = 0, i = 1;
		const XMPP::Resource lockedResource = account()->resourcePool()->lockedResource ( mRosterItem.jid () );

		// put default resource first
		items.append (i18n ("Automatic (best/default resource)"));

		account()->resourcePool()->findResources ( mRosterItem.jid (), availableResources );

		foreach(const XMPP::Resource& res, availableResources)
		{
			items.append ( res.name() );

			if ( res.name() == lockedResource.name() )
				activeItem = i;

			++i;
		}

		// now go through the string list and add the resources with their icons
		i = 0;
		foreach(const QString& str, items)
		{
			if( i == activeItem )
			{
				KAction *tmp = new KAction( this );
				tmp->setIcon( KIcon("dialog-ok") );
				tmp->setText( str);
				tmp->setObjectName( QString::number(i) );
				connect(tmp, SIGNAL(triggered(bool)), SLOT(slotSelectResource()));
				actionSelectResource->addAction(tmp);
			}
			else
			{
				/*
				 * Select icon, using bestResource() without lock for the automatic entry
				 * and the resources' respective status icons for the rest.
				 */
				QIcon iconSet ( !i ?
					protocol()->resourceToKOS ( account()->resourcePool()->bestResource ( mRosterItem.jid(), false ) ).iconFor ( account () ) : protocol()->resourceToKOS ( *availableResources.find(str) ).iconFor ( account () ));

				KAction *tmp = new KAction(this);
				tmp->setIcon( KIcon(iconSet) );
				tmp->setText( str );
				tmp->setObjectName( QString::number(i) );
				connect(tmp, SIGNAL(triggered(bool)), SLOT(slotSelectResource()));
				actionSelectResource->addAction ( tmp );
			}

			i++;
		}

	}

	actions->append( actionAuthorization );
	actions->append( actionSetAvailability );
	actions->append( actionSelectResource );
	
	
#if 0
	KAction *testAction = new KAction(i18n("Test action"), this);
	actionJingleAudioCall->setEnabled( true );
	actionCollection->append( testAction );

	KAction *actionJingleAudioCall = new KAction(i18n("Jingle Audio call"), this);
	connect(actionJingleAudioCall, SIGNAL(triggered(bool)), SLOT(slotJingleAudioCall()));
	
	KAction *actionJingleVideoCall = new KAction(i18n("Jingle Video call"), this);
	connect(actionJingleVideoCall, SIGNAL(triggered(bool)), SLOT(slotJingleVideoCall()));

	// Check if the current contact support jingle calls, also honor lock by default.
	JabberResource *bestResource = account()->resourcePool()->bestJabberResource( mRosterItem.jid() );
	actionJingleAudioCall->setEnabled( bestResource->features().canJingleAudio() );
	actionJingleVideoCall->setEnabled( bestResource->features().canJingleVideo() );
	
	actionCollection->append( actionJingleAudioCall );
	actionCollection->append( actionJingleVideoCall );
#endif

	// temporary action collection, used to apply Kiosk policy to the actions
	KActionCollection tempCollection((QObject*)0);
	tempCollection.addAction(QLatin1String("jabberContactAuthorizationMenu"), actionAuthorization);
	tempCollection.addAction(QLatin1String("contactSendAuth"), resendAuthAction);
	tempCollection.addAction(QLatin1String("contactRequestAuth"), requestAuthAction);
	tempCollection.addAction(QLatin1String("contactRemoveAuth"), removeAuthAction);
	tempCollection.addAction(QLatin1String("jabberContactSetAvailabilityMenu"), actionSetAvailability);
	tempCollection.addAction(QLatin1String("jabberContactSelectResource"), actionSelectResource);
	return actions;
}

void JabberContact::handleIncomingMessage (const XMPP::Message & message)
{
	QString viewPlugin;
	Kopete::Message *newMessage = 0L;

	kDebug (JABBER_DEBUG_GLOBAL) << "Received Message Type:" << message.type ();

	// fetch message manager
	JabberChatSession *mManager = manager ( message.from().resource (), Kopete::Contact::CanCreate );

	// evaluate notifications
	if ( message.type () != "error" )
	{
		if (!message.invite().isEmpty())
		{
			QString room=message.invite();
			QString originalBody=message.body().isEmpty() ? QString() :
					i18n( "The original message is : <i>\" %1 \"</i><br />" , Qt::escape(message.body()));
			QString mes=i18n("<qt><i>%1</i> has invited you to join the conference <b>%2</b><br />%3<br />"
					"If you want to accept and join, just <b>enter your nickname</b> and press OK.<br />"
							 "If you want to decline, press Cancel.</qt>",
					message.from().full(), room , originalBody);
			
			bool ok=false;
			QString futureNewNickName = KInputDialog::getText( i18n( "Invited to a conference - Jabber Plugin" ),
					mes, QString() , &ok , (mManager ? dynamic_cast<QWidget*>(mManager->view(false)) : 0) );
			if ( !ok || !account()->isConnected() || futureNewNickName.isEmpty() )
				return;
			
			XMPP::Jid roomjid(room);
			account()->client()->joinGroupChat( roomjid.domain() , roomjid.node() , futureNewNickName );
			return;
		}
		else if (message.body().isEmpty())
		// Then here could be event notifications
		{
			if (message.containsEvent ( XMPP::CancelEvent ) || (message.chatState() != XMPP::StateNone && message.chatState() != XMPP::StateComposing) )
				mManager->receivedTypingMsg ( this, false );
			else if (message.containsEvent ( XMPP::ComposingEvent )|| message.chatState() == XMPP::StateComposing )
				mManager->receivedTypingMsg ( this, true );
			if (message.containsEvent ( XMPP::DisplayedEvent ) )
				mManager->receivedEventNotification ( i18n("Message has been displayed") );
			else if (message.containsEvent ( XMPP::DeliveredEvent ) )
			{
				mManager->receivedEventNotification ( i18n("Message has been delivered") );
				mManager->receivedMessageState( message.eventId().toUInt(), Kopete::Message::StateSent );
				mSendsDeliveredEvent = true;
			}
			else if (message.containsEvent ( XMPP::OfflineEvent ) )
			{
				mManager->receivedEventNotification( i18n("Message stored on the server, contact offline") );
				mManager->receivedMessageState( message.eventId().toUInt(), Kopete::Message::StateSent );
			}
			else if (message.chatState() == XMPP::StateGone )
			{
				if(mManager->view( Kopete::Contact::CannotCreate ))
				{   //show an internal message if the user has not already closed his window
					Kopete::Message m=Kopete::Message ( this, mManager->members() );
					m.setPlainBody( i18n("%1 has ended his/her participation in the chat session.", metaContact()->displayName()) );
					m.setDirection( Kopete::Message::Internal );
					m.setImportance(Kopete::Message::Low);

					mManager->appendMessage ( m, message.from().resource () );
				}
			}

			// XEP-0184: Message Delivery Receipts
			if ( message.messageReceipt() == ReceiptReceived )
			{
				mManager->receivedEventNotification ( i18n("Message has been delivered") );
				mManager->receivedMessageState( message.messageReceiptId().toUInt(), Kopete::Message::StateSent );
				mSendsDeliveredEvent = true;
			}
		}
		else
		// Then here could be event notification requests
		{
			mRequestComposingEvent = message.containsEvent ( XMPP::ComposingEvent );
			mRequestOfflineEvent = message.containsEvent ( XMPP::OfflineEvent );
			mRequestDeliveredEvent = message.containsEvent ( XMPP::DeliveredEvent );
			mRequestDisplayedEvent = message.containsEvent ( XMPP::DisplayedEvent);

			// XEP-0184: Message Delivery Receipts
			mRequestReceiptDelivery = ( message.messageReceipt() == ReceiptRequest );
		}
	}

	/**
	 * Don't display empty messages, these were most likely just carrying
	 * event notifications or other payload.
	 */
	if ( message.body().isEmpty () && message.urlList().isEmpty () && !message.containsHTML() && message.xencrypted().isEmpty() && message.xsigned().isEmpty() )
		return;

	// determine message type
	if (message.type () == "chat")
		viewPlugin = "kopete_chatwindow";
	else
		viewPlugin = "kopete_emailwindow";

	Kopete::ContactPtrList contactList;
	contactList.append ( account()->myself () );

	// check for errors
	if ( message.type () == "error" )
	{
		mManager->receivedMessageState( message.id().toUInt(), Kopete::Message::StateError );
		newMessage = new Kopete::Message( this, contactList );
		newMessage->setTimestamp( message.timeStamp() );
		newMessage->setPlainBody( i18n("Your message could not be delivered: \"%1\", Reason: \"%2\"", 
										  message.body (), message.error().text ) );
		newMessage->setSubject( message.subject() );
		newMessage->setDirection( Kopete::Message::Inbound );
		newMessage->setRequestedPlugin( viewPlugin );
	}
	else
	{
		// store message id for outgoing notifications
		mLastReceivedMessageId = message.id ();

		// convert XMPP::Message into Kopete::Message
		// retrieve and reformat body
		QString body = message.body ();
		if( !message.xencrypted().isEmpty() )
		{
			kDebug ( JABBER_DEBUG_GLOBAL ) << "Received encrypted message";
			if (Kopete::PluginManager::self()->plugin("kopete_cryptography"))
			{
				kDebug( JABBER_DEBUG_GLOBAL ) << "Kopete cryptography plugin loaded";
				body = QString ("-----BEGIN PGP MESSAGE-----\n\n") + message.xencrypted () + QString ("\n-----END PGP MESSAGE-----\n");
			}
		}
		else if( !message.xsigned().isEmpty() )
		{
			kDebug ( JABBER_DEBUG_GLOBAL ) << "Received signed message";
			if (Kopete::PluginManager::self()->plugin("kopete_cryptography"))
			{
				kDebug( JABBER_DEBUG_GLOBAL ) << "Kopete cryptography plugin loaded";
				body = QString ("-----BEGIN PGP MESSAGE-----\n\n") + message.xsigned () + QString ("\n-----END PGP MESSAGE-----\n");
			}
		}

		if( message.containsHTML() )
		{
			kDebug ( JABBER_DEBUG_GLOBAL ) << "Received a xHTML message";
			newMessage = new Kopete::Message ( this, contactList );
			newMessage->setTimestamp( message.timeStamp() );
			newMessage->setHtmlBody( message.html().toString() );
			newMessage->setSubject( message.subject() );
			newMessage->setDirection( Kopete::Message::Inbound );
			newMessage->setRequestedPlugin( viewPlugin );
		}
		else if ( !body.isEmpty () )
		{
			kDebug ( JABBER_DEBUG_GLOBAL ) << "Received a plain text message";
			newMessage = new Kopete::Message ( this, contactList );
			newMessage->setTimestamp( message.timeStamp() );
			newMessage->setPlainBody( body );
			newMessage->setSubject( message.subject() );
			newMessage->setDirection( Kopete::Message::Inbound );
			newMessage->setRequestedPlugin( viewPlugin );
		}
	}

	// append message to (eventually new) manager and preselect the originating resource
	if ( newMessage )
	{
		mManager->appendMessage ( *newMessage, message.from().resource () );

		delete newMessage;
	}

	// append URLs as separate messages

	/*
	 * We need to copy it here because Iris returns a copy
	 * and we can't work with a returned copy in a for() loop.
	 */
	XMPP::UrlList urlList = message.urlList();

	foreach(XMPP::UrlList::const_reference xurl, urlList)
	{
		QString description = !xurl.desc().isEmpty() ? Qt::escape ( xurl.desc() ) : xurl.url();

		Kopete::Message msg ( this, contactList );
		msg.setTimestamp( message.timeStamp() );
		msg.setHtmlBody( QString ( "<a href=\"%1\">%2</a>" ).arg ( xurl.url(), description ) );
		msg.setSubject( message.subject() );
		msg.setDirection( Kopete::Message::Inbound );
		msg.setRequestedPlugin( viewPlugin );

		mManager->appendMessage ( msg, message.from().resource () );
	}
}

void JabberContact::slotCheckVCard ()
{
	QDateTime cacheDate;
	Kopete::Property cacheDateString = property ( protocol()->propVCardCacheTimeStamp );

	// don't do anything while we are offline
	if ( !account()->myself()->onlineStatus().isDefinitelyOnline () )
	{
		return;
	}
	
	if(!mDiscoDone)
	{
		if(transport()) //no need to disco if this is a legacy contact
			mDiscoDone = true;
		else if(!rosterItem().jid().node().isEmpty())
			mDiscoDone = true; //contact with an @ are not transport for sure
		else
		{
			mDiscoDone = true; //or it will happen twice, we don't want that.
			//disco to see if it's not a transport
			XMPP::JT_DiscoInfo *jt = new XMPP::JT_DiscoInfo(account()->client()->rootTask());
			QObject::connect(jt, SIGNAL(finished()),this, SLOT(slotDiscoFinished()));
			jt->get(rosterItem().jid(), QString());
			jt->go(true);
		}
	}


	// avoid warning if key does not exist in configuration file
	if ( cacheDateString.isNull () )
		cacheDate = QDateTime::currentDateTime().addDays ( -2 );
	else
		cacheDate = QDateTime::fromString ( cacheDateString.value().toString (), Qt::ISODate );

	kDebug ( JABBER_DEBUG_GLOBAL ) << "Cached vCard data for " << contactId () << " from " << cacheDate.toString ();

	if ( !mVCardUpdateInProgress && ( cacheDate.addDays ( 1 ) < QDateTime::currentDateTime () ) )
	{
		kDebug ( JABBER_DEBUG_GLOBAL ) << "Scheduling update.";

		mVCardUpdateInProgress = true;

		// current data is older than 24 hours, request a new one
		QTimer::singleShot ( account()->client()->getPenaltyTime () * 1000, this, SLOT (slotGetTimedVCard()) );
	}

}

void JabberContact::slotGetTimedVCard ()
{
	mVCardUpdateInProgress = false;

	// check if we are still connected - eventually we lost our connection in the meantime
	if ( !account()->myself()->onlineStatus().isDefinitelyOnline () )
	{
		// we are not connected, discard this update
		return;
	}
	
	if(!mDiscoDone)
	{
		if(transport()) //no need to disco if this is a legacy contact
			mDiscoDone = true;
		else if(!rosterItem().jid().node().isEmpty())
			mDiscoDone = true; //contact with an @ are not transport for sure
		else
		{
			//disco to see if it's not a transport
			XMPP::JT_DiscoInfo *jt = new XMPP::JT_DiscoInfo(account()->client()->rootTask());
			QObject::connect(jt, SIGNAL(finished()),this, SLOT(slotDiscoFinished()));
			jt->get(rosterItem().jid(), QString());
			jt->go(true);
		}
	}

	kDebug ( JABBER_DEBUG_GLOBAL ) << "Requesting vCard for " << contactId () << " from update timer.";

	mVCardUpdateInProgress = true;

	// request vCard
	XMPP::JT_VCard *task = new XMPP::JT_VCard ( account()->client()->rootTask () );
	// signal to ourselves when the vCard data arrived
	QObject::connect ( task, SIGNAL (finished()), this, SLOT (slotGotVCard()) );
	task->get ( mRosterItem.jid () );
	task->go ( true );

}

void JabberContact::slotGotVCard ()
{

	XMPP::JT_VCard * vCard = (XMPP::JT_VCard *) sender ();

	// update timestamp of last vCard retrieval
	if ( metaContact() && !metaContact()->isTemporary () )
	{
		setProperty ( protocol()->propVCardCacheTimeStamp, QDateTime::currentDateTime().toString ( Qt::ISODate ) );
	}

	mVCardUpdateInProgress = false;

	if ( !vCard->success() )
	{
		/*
		 * A vCard for the user does not exist or the
		 * request was unsuccessful or incomplete.
		 * The timestamp was already updated when
		 * requesting the vCard, so it's safe to
		 * just do nothing here.
		 */
		return;
	}

	setPropertiesFromVCard ( vCard->vcard () );

}

void JabberContact::slotCheckLastActivity ( Kopete::Contact *, const Kopete::OnlineStatus &newStatus, const Kopete::OnlineStatus &oldStatus )
{

	/*
	 * Checking the last activity only makes sense if a contact is offline.
	 * So, this check should only be done in the following cases:
	 * - Kopete goes online for the first time and this contact is offline, or
	 * - Kopete is already online and this contact went offline.
	 *
	 * Since Kopete already takes care of maintaining the lastSeen property
	 * if the contact changes its state while we are online, we don't need
	 * to query its activity after we are already connected.
	 */

	if ( onlineStatus().isDefinitelyOnline () )
	{
		// Kopete already deals with lastSeen if the contact is online
		return;
	}
	
	if ( ( oldStatus.status () == Kopete::OnlineStatus::Connecting ) && newStatus.isDefinitelyOnline () )
	{
		kDebug ( JABBER_DEBUG_GLOBAL ) << "Scheduling request for last activity for " << mRosterItem.jid().bare ();

		QTimer::singleShot ( account()->client()->getPenaltyTime () * 1000, this, SLOT (slotGetTimedLastActivity()) );
	}

}

void JabberContact::slotGetTimedLastActivity ()
{
	/*
	 * We have been called from @ref slotCheckLastActivity.
	 * We could have lost our connection in the meantime,
	 * so make sure we are online. Additionally, the contact
	 * itself could have gone online, so make sure it is
	 * still offline. (otherwise the last seen property is
	 * maintained by Kopete)
	 */

	if ( onlineStatus().isDefinitelyOnline () )
	{
		// Kopete already deals with setting lastSeen if the contact is online
		return;
	}

	if ( account()->myself()->onlineStatus().isDefinitelyOnline () )
	{
		kDebug ( JABBER_DEBUG_GLOBAL ) << "Requesting last activity from timer for " << mRosterItem.jid().bare ();

		JT_GetLastActivity *task = new JT_GetLastActivity ( account()->client()->rootTask () );
		QObject::connect ( task, SIGNAL (finished()), this, SLOT (slotGotLastActivity()) );
		task->get ( mRosterItem.jid () );
		task->go ( true );
	}

}

void JabberContact::slotGotLastActivity ()
{
	JT_GetLastActivity *task = (JT_GetLastActivity *) sender ();

	if ( task->success () )
	{
		setProperty ( protocol()->propLastSeen, QDateTime::currentDateTime().addSecs ( -task->seconds () ) );
		if( !task->message().isEmpty() )
		{
			setStatusMessage( task->message() );
		}
	}

}

void JabberContact::slotSendVCard()
{
	XMPP::VCard vCard;
	XMPP::VCard::AddressList addressList;
	XMPP::VCard::EmailList emailList;
	XMPP::VCard::PhoneList phoneList;

	if ( !account()->isConnected () )
	{
		account()->errorConnectFirst ();
		return;
	}

	// General information
	vCard.setNickName (property(protocol()->propNickName).value().toString());
	vCard.setFullName (property(protocol()->propFullName).value().toString());
	vCard.setJid (property(protocol()->propJid).value().toString());
	vCard.setBdayStr (property(protocol()->propBirthday).value().toString());
	vCard.setTimezone (property(protocol()->propTimezone).value().toString());
	vCard.setUrl (property(protocol()->propHomepage).value().toString());

	// home address tab
	XMPP::VCard::Address homeAddress;

	homeAddress.home = true;
	homeAddress.street = property(protocol()->propHomeStreet).value().toString();
	homeAddress.extaddr = property(protocol()->propHomeExtAddr).value().toString();
	homeAddress.pobox = property(protocol()->propHomePOBox).value().toString();
	homeAddress.locality = property(protocol()->propHomeCity).value().toString();
	homeAddress.pcode = property(protocol()->propHomePostalCode).value().toString();
	homeAddress.country = property(protocol()->propHomeCountry).value().toString();

	// work address tab
	XMPP::VCard::Address workAddress;

	workAddress.work = true;
	workAddress.street = property(protocol()->propWorkStreet).value().toString();
	workAddress.extaddr = property(protocol()->propWorkExtAddr).value().toString();
	workAddress.pobox = property(protocol()->propWorkPOBox).value().toString();
	workAddress.locality = property(protocol()->propWorkCity).value().toString();
	workAddress.pcode = property(protocol()->propWorkPostalCode).value().toString();
	workAddress.country = property(protocol()->propWorkCountry).value().toString();

	addressList.append(homeAddress);
	addressList.append(workAddress);

	vCard.setAddressList(addressList);

	// home email
	XMPP::VCard::Email homeEmail;

	homeEmail.home = true;
	homeEmail.userid = property(protocol()->propEmailAddress).value().toString();

	// work email
	XMPP::VCard::Email workEmail;

	workEmail.work = true;
	workEmail.userid = property(protocol()->propWorkEmailAddress).value().toString();

	emailList.append(homeEmail);
	emailList.append(workEmail);

	vCard.setEmailList(emailList);

	// work information tab
	XMPP::VCard::Org org;
	org.name = property(protocol()->propCompanyName).value().toString();
	org.unit = property(protocol()->propCompanyDepartement).value().toString().split(',');
	vCard.setOrg(org);
	vCard.setTitle (property(protocol()->propCompanyPosition).value().toString());
	vCard.setRole (property(protocol()->propCompanyRole).value().toString());

	// phone numbers tab
	XMPP::VCard::Phone phoneHome;
	phoneHome.home = true;
	phoneHome.number = property(protocol()->propPrivatePhone).value().toString();

	XMPP::VCard::Phone phoneWork;
	phoneWork.work = true;
	phoneWork.number = property(protocol()->propWorkPhone).value().toString();

	XMPP::VCard::Phone phoneFax;
	phoneFax.fax = true;
	phoneFax.number = property(protocol()->propPhoneFax).value().toString();

	XMPP::VCard::Phone phoneCell;
	phoneCell.cell = true;
	phoneCell.number = property(protocol()->propPrivateMobilePhone).value().toString();

	phoneList.append(phoneHome);
	phoneList.append(phoneWork);
	phoneList.append(phoneFax);
	phoneList.append(phoneCell);

	vCard.setPhoneList(phoneList);

	// about tab
	vCard.setDesc(property(protocol()->propAbout).value().toString());

	// Set contact photo as a binary value (if he has set a photo)
	if( hasProperty( protocol()->propPhoto.key() ) )
	{
		QString photoPath = property( protocol()->propPhoto ).value().toString();
		QImage image( photoPath );
		QByteArray ba;
		QBuffer buffer( &ba );
		buffer.open( QIODevice::WriteOnly );
		image.save( &buffer, "PNG" );
		vCard.setPhoto( ba );
	}

	vCard.setVersion("3.0");
	vCard.setProdId("Kopete");

	XMPP::JT_VCard *task = new XMPP::JT_VCard (account()->client()->rootTask ());
	// signal to ourselves when the vCard data arrived
	QObject::connect (task, SIGNAL (finished()), this, SLOT (slotSentVCard()));
	task->set (rosterItem().jid(), vCard);
	task->go (true);
}

void JabberContact::setPhoto( const QString &photoPath )
{
	QImage contactPhoto(photoPath);
	QString newPhotoPath = photoPath;
	if(contactPhoto.width() > 96 || contactPhoto.height() > 96)
	{
		// Save image to a new location if the image isn't the correct format.
		QString newLocation( KStandardDirs::locateLocal( "appdata", "jabberphotos/"+ KUrl(photoPath).fileName().toLower() ) );
	
		// Scale and crop the picture.
		contactPhoto = contactPhoto.scaled( 96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation );
		// crop image if not square
		if(contactPhoto.width() < contactPhoto.height()) 
			contactPhoto = contactPhoto.copy((contactPhoto.width()-contactPhoto.height())/2, 0, 96, 96);
		else if (contactPhoto.width() > contactPhoto.height())
			contactPhoto = contactPhoto.copy(0, (contactPhoto.height()-contactPhoto.width())/2, 96, 96);
	
		// Use the cropped/scaled image now.
		if(!contactPhoto.save(newLocation, "PNG"))
			newPhotoPath.clear();
		else
			newPhotoPath = newLocation;
	}
	else if (contactPhoto.width() < 32 || contactPhoto.height() < 32)
	{
		// Save image to a new location if the image isn't the correct format.
		QString newLocation( KStandardDirs::locateLocal( "appdata", "jabberphotos/"+ KUrl(photoPath).fileName().toLower() ) );
	
		// Scale and crop the picture.
		contactPhoto = contactPhoto.scaled( 32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation );
		// crop image if not square
		if(contactPhoto.width() < contactPhoto.height())
			contactPhoto = contactPhoto.copy((contactPhoto.width()-contactPhoto.height())/2, 0, 32, 32);
		else if (contactPhoto.width() > contactPhoto.height())
			contactPhoto = contactPhoto.copy(0, (contactPhoto.height()-contactPhoto.width())/2, 32, 32);
	
		// Use the cropped/scaled image now.
		if(!contactPhoto.save(newLocation, "PNG"))
			newPhotoPath.clear();
		else
			newPhotoPath = newLocation;
	}
	else if (contactPhoto.width() != contactPhoto.height())
	{
		// Save image to a new location if the image isn't the correct format.
		QString newLocation( KStandardDirs::locateLocal( "appdata", "jabberphotos/"+ KUrl(photoPath).fileName().toLower() ) );

		if(contactPhoto.width() < contactPhoto.height())
			contactPhoto = contactPhoto.copy((contactPhoto.width()-contactPhoto.height())/2, 0, contactPhoto.height(), contactPhoto.height());
		else if (contactPhoto.width() > contactPhoto.height())
			contactPhoto = contactPhoto.copy(0, (contactPhoto.height()-contactPhoto.width())/2, contactPhoto.height(), contactPhoto.height());

		// Use the cropped/scaled image now.
		if(!contactPhoto.save(newLocation, "PNG"))
			newPhotoPath.clear();
		else
			newPhotoPath = newLocation;
	}

	setProperty( protocol()->propPhoto, newPhotoPath );
}

void JabberContact::slotSentVCard ()
{
	
}

void JabberContact::slotChatSessionDeleted ( QObject *sender )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Message manager deleted, collecting the pieces...";

	JabberChatSession *manager = static_cast<JabberChatSession *>(sender);

	mManagers.removeAll ( manager );

}

JabberChatSession *JabberContact::manager ( Kopete::ContactPtrList chatMembers, Kopete::Contact::CanCreateFlags canCreate )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "called, canCreate: " << canCreate;

	Kopete::ChatSession *_manager = Kopete::ChatSessionManager::self()->findChatSession ( account()->myself(), chatMembers, protocol() );
	JabberChatSession *manager = dynamic_cast<JabberChatSession*>( _manager );

	/*
	 * If we didn't find a message manager for this contact,
	 * instantiate a new one if we are allowed to. (otherwise return 0)
	 */
	if ( !manager &&  canCreate )
	{
		XMPP::Jid jid = rosterItem().jid();

		/*
		 * If we have no hardwired JID, set any eventually
		 * locked resource as preselected resource.
		 * If there is no locked resource, the resource field
		 * will stay empty.
		 */
		if ( jid.resource().isEmpty () )
			jid = jid.withResource ( account()->resourcePool()->lockedResource ( jid ).name () );

		kDebug(JABBER_DEBUG_GLOBAL) << "No manager found, creating a new one with resource '" << jid.resource () << "'";

		manager = new JabberChatSession ( protocol(), static_cast<JabberBaseContact *>(account()->myself()), chatMembers, jid.resource () );
		connect ( manager, SIGNAL (destroyed(QObject*)), this, SLOT (slotChatSessionDeleted(QObject*)) );
		mManagers.append ( manager );
	}

	return manager;

}

Kopete::ChatSession *JabberContact::manager ( Kopete::Contact::CanCreateFlags canCreate )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "called, canCreate: " << canCreate;

	Kopete::ContactPtrList chatMembers;
	chatMembers.append ( this );

	return manager ( chatMembers, canCreate );

}

JabberChatSession *JabberContact::manager ( const QString &resource, Kopete::Contact::CanCreateFlags canCreate )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "called, canCreate: " << canCreate << ", Resource: '" << resource << "'";

	/*
	 * First of all, see if we already have a manager matching
	 * the requested resource or if there are any managers with
	 * an empty resource.
	 */
	if ( !resource.isEmpty () )
	{
		QList<JabberChatSession*>::iterator it = mManagers.begin();
		QList<JabberChatSession*>::iterator end = mManagers.end();
		for ( ; it != end ; ++it )
		{
			JabberChatSession *mManager = *it;
			if ( account()->mergeMessages () || mManager->resource().isEmpty () || ( mManager->resource () == resource ) )
			{
				// we found a matching manager, return this one
				kDebug(JABBER_DEBUG_GLOBAL) << "Found an existing message manager for this resource.";
				return mManager;
			}
		}

		kDebug(JABBER_DEBUG_GLOBAL) << "No manager found for this resource, creating a new one.";

		/*
		 * If we have come this far, we were either supposed to create
		 * a manager with a preselected resource but have found
		 * no available manager. (not even one with an empty resource)
		 * This means, we will have to create a new one with a
		 * preselected resource.
		 */
		Kopete::ContactPtrList chatmembers;
		chatmembers.append ( this );
		JabberChatSession *manager = new JabberChatSession ( protocol(),
																   static_cast<JabberBaseContact *>(account()->myself()),
																   chatmembers, resource );
		connect ( manager, SIGNAL (destroyed(QObject*)), this, SLOT (slotChatSessionDeleted(QObject*)) );
		mManagers.append ( manager );

		return manager;
	}

	kDebug(JABBER_DEBUG_GLOBAL) << "Resource is empty, grabbing first available manager.";

	/*
	 * The resource is empty, so just return first available manager.
	 */
	return dynamic_cast<JabberChatSession *>( manager ( canCreate ) );

}

void JabberContact::deleteContact ()
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Removing user " << contactId ();

	if ( !account()->isConnected () )
	{
		account()->errorConnectFirst ();
		return;
	}
	
	/*
	* Follow the recommendation of
	*  JEP-0162: Best Practices for Roster and Subscription Management
	* http://www.jabber.org/jeps/jep-0162.html#removal
	*/

	bool remove_from_roster=false;
	
	if( mRosterItem.subscription().type() == XMPP::Subscription::Both || mRosterItem.subscription().type() == XMPP::Subscription::From )
	{
		int result = KMessageBox::questionYesNoCancel (Kopete::UI::Global::mainWidget(),
		 				i18n ( "Do you also want to remove user %1's authorization to see your status?" , 
						  mRosterItem.jid().bare () ), i18n ("Notification"),
						KStandardGuiItem::del (), KGuiItem( i18n("Keep") ),KStandardGuiItem::cancel(), "JabberRemoveAuthorizationOnDelete" );
		if(result == KMessageBox::Yes )
			remove_from_roster = true;
		else if( result == KMessageBox::Cancel)
			return;
	}
	else if( mRosterItem.subscription().type() == XMPP::Subscription::None || mRosterItem.subscription().type() == XMPP::Subscription::To )
		remove_from_roster = true;
	
	if( remove_from_roster )
	{
		XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( account()->client()->rootTask () );
		rosterTask->remove ( mRosterItem.jid () );
		rosterTask->go ( true );
	}
	else
	{
		sendSubscription("unsubscribe");

		XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( account()->client()->rootTask () );
		rosterTask->set ( mRosterItem.jid (), QString() , QStringList() );
		rosterTask->go (true);
	}

}

void JabberContact::sync ( unsigned int )
{
	// if we are offline or this is a temporary contact or we should not synch, don't bother
	if ( dontSync () || !account()->isConnected () || metaContact()->isTemporary () || metaContact() == Kopete::ContactList::self()->myself() )
		return;
	
	kDebug ( JABBER_DEBUG_GLOBAL ) << contactId () /*<< " - " <<kdBacktrace()*/;

	if(!m_syncTimer)
	{
		m_syncTimer=new QTimer(this);
		connect(m_syncTimer, SIGNAL(timeout()) , this , SLOT(slotDelayedSync()));
	}
	m_syncTimer->setSingleShot(true);
	m_syncTimer->start(2*1000);

	/*
	the sync operation is delayed, because when we are doing a move to group operation,
	kopete first add the contact to the group, then removes it.
	Theses two operations should anyway be done in only one pass.
	
	if there is two jabber contact in one metacontact, this may result in an infinite change of
	groups between theses two contacts, and the server is being flooded.
	*/
}

void JabberContact::slotDelayedSync( )
{
	m_syncTimer->deleteLater();
	m_syncTimer=0L;
	// if we are offline or this is a temporary contact or we should not synch, don't bother
	if ( dontSync () || !account()->isConnected () || metaContact()->isTemporary () )
		return;
	
	bool changed=metaContact()->displayName() != mRosterItem.name();
	

	QStringList groups;
	Kopete::GroupList groupList = metaContact ()->groups ();

	kDebug ( JABBER_DEBUG_GLOBAL ) << "Synchronizing contact " << contactId ();

	foreach ( Kopete::Group * g, groupList )
	{
		if ( g->type () == Kopete::Group::Normal )
			groups += g->displayName ();
		else if ( g->type () == Kopete::Group::TopLevel )
			groups += QString();
	}

	if(groups.size() == 1 && groups.at(0).isEmpty())
		groups.clear();

	if(mRosterItem.groups() != groups)
	{
		changed=true;
		mRosterItem.setGroups ( groups );
	}
	
	if(!changed)
	{
		kDebug ( JABBER_DEBUG_GLOBAL ) << "contact has not changed,  abort sync";
		return;
	}

	XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( account()->client()->rootTask () );

	rosterTask->set ( mRosterItem.jid (), metaContact()->displayName (), mRosterItem.groups () );
	rosterTask->go (true);

}

void JabberContact::sendFile ( const KUrl &sourceURL, const QString &/*fileName*/, uint /*fileSize*/ )
{
	QString filePath;

	// if the file location is null, then get it from a file open dialog
	if ( !sourceURL.isValid () )
		filePath = KFileDialog::getOpenFileName( KUrl(), "*", 0L, i18n ( "Kopete File Transfer" ) );
	else
		filePath = sourceURL.path(KUrl::RemoveTrailingSlash);

	QFile file ( filePath );

	if ( file.exists () )
	{
		// send the file
		new JabberFileTransfer ( account (), this, filePath );
	}

}


void JabberContact::slotSendAuth ()
{

	kDebug (JABBER_DEBUG_GLOBAL) << "(Re)send auth " << contactId ();

	sendSubscription ("subscribed");

}

void JabberContact::slotRequestAuth ()
{

	kDebug (JABBER_DEBUG_GLOBAL) << "(Re)request auth " << contactId ();

	sendSubscription ("subscribe");

}

void JabberContact::slotRemoveAuth ()
{

	kDebug (JABBER_DEBUG_GLOBAL) << "Remove auth " << contactId ();

	sendSubscription ("unsubscribed");

}

void JabberContact::sendSubscription ( const QString& subType )
{

	if ( !account()->isConnected () )
	{
		account()->errorConnectFirst ();
		return;
	}

	XMPP::JT_Presence * task = new XMPP::JT_Presence ( account()->client()->rootTask () );

	task->sub ( mRosterItem.jid().full (), subType );
	task->go ( true );

}

void JabberContact::slotSelectResource ()
{
	int currentItem = QString ( static_cast<const KAction *>( sender() )->objectName () ).toUInt ();

	/*
	 * Warn the user if there is already an active chat window.
	 * The resource selection will only apply for newly opened
	 * windows.
	 */
	if ( manager ( Kopete::Contact::CannotCreate ) != 0 )
	{
		KMessageBox::queuedMessageBox ( Kopete::UI::Global::mainWidget (),
										KMessageBox::Information,
										i18n ("You have preselected a resource for contact %1, "
										"but you still have open chat windows for this contact. "
										"The preselected resource will only apply to newly opened "
										"chat windows.",  contactId () ),
										i18n ("Jabber Resource Selector") );
	}

	if (currentItem == 0)
	{
		kDebug (JABBER_DEBUG_GLOBAL) << "Removing active resource, trusting bestResource().";

		account()->resourcePool()->removeLock ( rosterItem().jid() );
	}
	else
	{
		// use iconText() instead of text(), because we need stripped value without '&'
		QString selectedResource = static_cast<const KAction *>(sender())->iconText();

		kDebug (JABBER_DEBUG_GLOBAL) << "Moving to resource " << selectedResource;

		account()->resourcePool()->lockToResource ( rosterItem().jid() , XMPP::Resource ( selectedResource ) );
	}

}

void JabberContact::sendPresence ( const XMPP::Status status )
{

	if ( !account()->isConnected () )
	{
		account()->errorConnectFirst ();
		return;
	}

	XMPP::Status newStatus = status;

	// honor our priority
	if(newStatus.isAvailable())
		newStatus.setPriority ( account()->configGroup()->readEntry ( "Priority", 5 ) );

	XMPP::JT_Presence * task = new XMPP::JT_Presence ( account()->client()->rootTask () );

	task->pres ( bestAddress (), newStatus);
	task->go ( true );

}


void JabberContact::slotStatusOnline ()
{

	XMPP::Status status;
	status.setShow("");

	sendPresence ( status );

}

void JabberContact::slotStatusChatty ()
{

	XMPP::Status status;
	status.setShow ("chat");

	sendPresence ( status );

}

void JabberContact::slotStatusAway ()
{

	XMPP::Status status;
	status.setShow ("away");

	sendPresence ( status );

}

void JabberContact::slotStatusXA ()
{

	XMPP::Status status;
	status.setShow ("xa");

	sendPresence ( status );

}

void JabberContact::slotStatusDND ()
{

	XMPP::Status status;
	status.setShow ("dnd");

	sendPresence ( status );


}

void JabberContact::slotStatusInvisible ()
{

	XMPP::Status status;
	status.setIsAvailable( false );

	sendPresence ( status );

}

bool JabberContact::isContactRequestingEvent( XMPP::MsgEvent event )
{
	if ( event == OfflineEvent )
		return mRequestOfflineEvent;
	else if ( event == DeliveredEvent )
		return mRequestDeliveredEvent;
	else if ( event == DisplayedEvent )
		return mRequestDisplayedEvent;
	else if ( event == ComposingEvent )
		return mRequestComposingEvent;
	else if ( event == CancelEvent )
		return mRequestComposingEvent;
	else
		return false;
}

bool JabberContact::isContactRequestingReceiptDelivery()
{
	return mRequestReceiptDelivery;
}

QString JabberContact::lastReceivedMessageId () const
{
	return mLastReceivedMessageId;
}

void JabberContact::slotDiscoFinished( )
{
	mDiscoDone = true;
	JT_DiscoInfo *jt = (JT_DiscoInfo *)sender();
	
	bool is_transport=false;
	QString tr_type;

	if ( jt->success() )
 	{
		QList<XMPP::DiscoItem::Identity> identities = jt->item().identities();
		QList<XMPP::DiscoItem::Identity>::Iterator it;
		for ( it = identities.begin(); it != identities.end(); ++it )
		{
			XMPP::DiscoItem::Identity ident=*it;
			if(ident.category == "gateway")
			{
				is_transport=true;
				tr_type=ident.type;
				//name=ident.name;
		
				break;  //(we currently only support gateway)
			}
			else if (ident.category == "service")
			{
				//The ApaSMSAgent is reporting itself as service (instead of gateway) which is broken.
				//we anyway support it.  See bug  127811
				if(ident.type == "sms")
				{
					is_transport=true;
					tr_type=ident.type;
				}
			}
		}
 	}

	if(is_transport && !transport()) 
 	{   //ok, we are not a contact, we are a transport....
		
		XMPP::RosterItem ri = rosterItem();
		Kopete::MetaContact *mc=metaContact();
		JabberAccount *parentAccount=account();
		Kopete::OnlineStatus status=onlineStatus();
		
		kDebug(JABBER_DEBUG_GLOBAL) << ri.jid().full() << " is not a contact but a gateway   - " << this;
		
		if( Kopete::AccountManager::self()->findAccount( protocol()->pluginId() , account()->accountId() + '/' + ri.jid().bare() ) )
		{
			kDebug(JABBER_DEBUG_GLOBAL) << "oops, transport already exists, abort operation ";
			return;
		}
		
		delete this; //we are not a contact i said !
		
		if(mc->contacts().count() == 0)
			Kopete::ContactList::self()->removeMetaContact( mc );
		
		//we need to create the transport when 'this' is already deleted, so transport->myself() will not conflict with it
		JabberTransport *transport = new JabberTransport( parentAccount , ri , tr_type );
		if(!Kopete::AccountManager::self()->registerAccount( transport ))
			return;
		
		transport->myself()->setOnlineStatus( status ); //push back the online status
		return;
	}
}

#ifdef JINGLE_SUPPORT
void JabberContact::showSessionsGui()
{
	account()->jingleCallsManager()->showCallsGui();
}

void JabberContact::startJingleSession()
{
	startJingleVideoCall(); //Only to show the message.

	account()->jingleCallsManager()->startNewSession(/*to*/fullAddress());
	account()->jingleCallsManager()->showCallsGui();
}

void JabberContact::startJingleAudioCall()
{
	startJingleVideoCall(); //Only to show the message.

	//There should also have a list of jingle features supported by the responder client.
	//--> Will be done in Iris as iris should now what features are supported by the responder client.
	account()->jingleCallsManager()->startNewSession(/*to*/fullAddress());
}

void JabberContact::startJingleVideoCall()
{
	kDebug() << "Start a Jingle Session";

	//JingleCallsManager should manage messages itself.
	/*XMPP::Jid jid = rosterItem().jid();
	JabberResource *bestResource;
	XMPP::Resource resource = account()->resourcePool()->bestJabberResource( jid )->resource();
	if (&resource)
	{
		JabberChatSession *mManager = manager ( resource.name(), Kopete::Contact::CanCreate );
		Kopete::Message m = Kopete::Message ( this, mManager->members());
		m.setPlainBody( i18n("Starting a Jingle session with %1", metaContact()->displayName()) );
		m.setDirection( Kopete::Message::Internal );
		mManager->appendMessage ( m, resource.name() );
	}
	else
	{
		if (!manager(CannotCreate))
			return;
		Kopete::Message msg;
		msg.setPlainBody( "Failed to start a Jingle session, is your contact Online ?" );
		manager(CannotCreate)->appendMessage( msg );
		;
		//FIXME:How to write a message in the chat dialog when contact is offline ?
	}*/

	//TODO:implement me !
}
#endif

#ifdef LIBJINGLE_SUPPORT

void JabberContact::makeLibjingleCallAction()
{
	account()->makeLibjingleCall(contactId());
}

#endif

#include "jabbercontact.moc"
