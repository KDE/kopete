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
#include "jabber_protocol_debug.h"
#include <KLocalizedString>
#include <kmessagebox.h>
#include <QAction>
#include <kactionmenu.h>
#include <kicon.h>

#include <kio/netaccess.h>
#include <qinputdialog.h>
#include <kopeteview.h>
#include <QStandardPaths>
#include <QFileDialog>

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
	: JabberBaseContact ( rosterItem, _account, mc, legacyId)  , mDiscoDone(false), m_syncTimer(nullptr)
{
	///*account()->client()->clientStream()->*/XMPP::setDebug((Debug*) new TD());
	qCDebug(JABBER_PROTOCOL_LOG) << contactId() << "  is created  - " << this;
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
	qCDebug(JABBER_PROTOCOL_LOG) << contactId() << "  is destroyed  - " << this;
}

QList<QAction *> *JabberContact::customContextMenuActions ()
{

	QList<QAction *> *actions = new QList<QAction*>();

    KActionMenu *actionAuthorization = new KActionMenu ( QIcon::fromTheme(QStringLiteral("network-connect")), i18n ("Authorization"), this);

	QAction *resendAuthAction, *requestAuthAction, *removeAuthAction;
	
	resendAuthAction = new QAction( this );
    resendAuthAction->setIcon( QIcon::fromTheme(QStringLiteral("mail-forward")) );
	resendAuthAction->setText( i18n ("(Re)send Authorization To") );
	resendAuthAction->setEnabled( mRosterItem.subscription().type() == XMPP::Subscription::To || mRosterItem.subscription().type() == XMPP::Subscription::None );
	connect(resendAuthAction, SIGNAL(triggered(bool)), SLOT(slotSendAuth()));
	actionAuthorization->addAction(resendAuthAction);

	requestAuthAction = new QAction( this );
    requestAuthAction->setIcon( QIcon::fromTheme(QStringLiteral("mail-reply-sender")) );
	requestAuthAction->setText( i18n ("(Re)request Authorization From") );
	requestAuthAction->setEnabled( mRosterItem.subscription().type() == XMPP::Subscription::From || mRosterItem.subscription().type() == XMPP::Subscription::None );
	connect(requestAuthAction, SIGNAL(triggered(bool)), SLOT(slotRequestAuth()));
	actionAuthorization->addAction(requestAuthAction);
	
	removeAuthAction = new QAction( this );
    removeAuthAction->setIcon( QIcon::fromTheme(QStringLiteral("edit-delete")) );
	removeAuthAction->setText( i18n ("Remove Authorization From") );
	removeAuthAction->setEnabled( mRosterItem.subscription().type() == XMPP::Subscription::Both || mRosterItem.subscription().type() == XMPP::Subscription::From );
	connect(removeAuthAction, SIGNAL(triggered(bool)), SLOT(slotRemoveAuth()));
	actionAuthorization->addAction(removeAuthAction);

#ifdef LIBJINGLE_SUPPORT

	if ( account()->enabledLibjingle() ) {

		QAction *libjingleCallAction;
		libjingleCallAction = new QAction( this );
        libjingleCallAction->setIcon( (QIcon::fromTheme("voicecall") ) );
		libjingleCallAction->setText( i18n ("Call contact") );
		libjingleCallAction->setEnabled( account()->supportLibjingle(contactId()) );
		connect(libjingleCallAction, SIGNAL(triggered(bool)), SLOT(makeLibjingleCallAction()));
		actions->append(libjingleCallAction);

	}

#endif

    KActionMenu *actionSetAvailability = new KActionMenu ( KIcon(QStringLiteral("user-identity"), 0, QStringList() << QString() << QStringLiteral("user-online")), i18n ("Set Availability"), this );

#define QAction(status, text, name, slot) \
	{ QAction *tmp = new QAction(this); \
    tmp->setIcon( QIcon((status).iconFor(this))); \
	tmp->setText( text ); \
	connect(tmp, SIGNAL(triggered(bool)), (slot));\
	actionSetAvailability->addAction(tmp); }

    QAction( protocol()->JabberKOSOnline, i18n ("Online"),         "actionOnline", SLOT(slotStatusOnline()) );
    QAction( protocol()->JabberKOSChatty, i18n ("Free to Chat"),   "actionChatty", SLOT(slotStatusChatty()) );
    QAction( protocol()->JabberKOSAway,   i18n ("Away"),           "actionAway",   SLOT(slotStatusAway()) );
    QAction( protocol()->JabberKOSXA,     i18n ("Extended Away"),  "actionXA",     SLOT(slotStatusXA()) );
    QAction( protocol()->JabberKOSDND,    i18n ("Do Not Disturb"), "actionDND",    SLOT(slotStatusDND()) );
    QAction( protocol()->JabberKOSInvisible, i18n ("Invisible"),   "actionInvisible", SLOT(slotStatusInvisible()) );

#undef QAction

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
				QAction *tmp = new QAction( this );
                tmp->setIcon( QIcon::fromTheme(QStringLiteral("dialog-ok")) );
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

				QAction *tmp = new QAction(this);
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
	QAction *testAction = new QAction(i18n("Test action"), this);
	actionJingleAudioCall->setEnabled( true );
	actionCollection->append( testAction );

	QAction *actionJingleAudioCall = new QAction(i18n("Jingle Audio call"), this);
	connect(actionJingleAudioCall, SIGNAL(triggered(bool)), SLOT(slotJingleAudioCall()));
	
	QAction *actionJingleVideoCall = new QAction(i18n("Jingle Video call"), this);
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
	tempCollection.addAction(QStringLiteral("jabberContactAuthorizationMenu"), actionAuthorization);
	tempCollection.addAction(QStringLiteral("contactSendAuth"), resendAuthAction);
	tempCollection.addAction(QStringLiteral("contactRequestAuth"), requestAuthAction);
	tempCollection.addAction(QStringLiteral("contactRemoveAuth"), removeAuthAction);
	tempCollection.addAction(QStringLiteral("jabberContactSetAvailabilityMenu"), actionSetAvailability);
	tempCollection.addAction(QStringLiteral("jabberContactSelectResource"), actionSelectResource);
	return actions;
}

void JabberContact::handleIncomingMessage (const XMPP::Message & message)
{
	QString viewPlugin;
	Kopete::Message *newMessage = nullptr;

	qDebug (JABBER_PROTOCOL_LOG) << "Received Message Type:" << message.type ();

	// fetch message manager
	JabberChatSession *mManager = manager ( message.from().resource (), Kopete::Contact::CanCreate );

	// evaluate notifications
	if ( message.type () != QLatin1String("error") )
	{
		if (!message.invite().isEmpty())
		{
			QString room=message.invite();
			QString originalBody=message.body().isEmpty() ? QString() :
					i18n( "The original message is : <i>\" %1 \"</i><br />" , message.body().toHtmlEscaped());
			QString mes=i18n("<qt><i>%1</i> has invited you to join the conference <b>%2</b><br />%3<br />"
					"If you want to accept and join, just <b>enter your nickname</b> and press OK.<br />"
							 "If you want to decline, press Cancel.</qt>",
					message.from().full(), room , originalBody);
			
			bool ok=false;
            QString futureNewNickName = QInputDialog::getText( (mManager ? dynamic_cast<QWidget*>(mManager->view(false)) : 0), i18n( "Invited to a conference - Jabber Plugin" ),
                    mes,QLineEdit::Normal, QString() , &ok  );
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
				JabberResource *jresource = account()->resourcePool()->getJabberResource(message.from(), message.from().resource());
				// getJabberResource() can returns best resource so verify it is same as in message
				if (jresource && jresource->resource().name().toLower() == message.from().resource().toLower())
					jresource->setSendsDeliveredEvent(true);
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
	if (message.type () == QLatin1String("chat"))
		viewPlugin = QStringLiteral("kopete_chatwindow");
	else
		viewPlugin = QStringLiteral("kopete_emailwindow");

	Kopete::ContactPtrList contactList;
	contactList.append ( account()->myself () );

	// check for errors
	if ( message.type () == QLatin1String("error") )
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
			qCDebug(JABBER_PROTOCOL_LOG) << "Received encrypted message";
			if (Kopete::PluginManager::self()->plugin(QStringLiteral("kopete_cryptography")))
			{
				qCDebug(JABBER_PROTOCOL_LOG) << "Kopete cryptography plugin loaded";
				body = QStringLiteral ("-----BEGIN PGP MESSAGE-----\n\n") + message.xencrypted () + QStringLiteral ("\n-----END PGP MESSAGE-----\n");
			}
		}
		else if( !message.xsigned().isEmpty() )
		{
			qCDebug(JABBER_PROTOCOL_LOG) << "Received signed message";
			if (Kopete::PluginManager::self()->plugin(QStringLiteral("kopete_cryptography")))
			{
				qCDebug(JABBER_PROTOCOL_LOG) << "Kopete cryptography plugin loaded";
				body = QStringLiteral ("-----BEGIN PGP MESSAGE-----\n\n") + message.xsigned () + QStringLiteral ("\n-----END PGP MESSAGE-----\n");
			}
		}

		if( message.containsHTML() )
		{
			qCDebug(JABBER_PROTOCOL_LOG) << "Received a xHTML message";
			newMessage = new Kopete::Message ( this, contactList );
			newMessage->setTimestamp( message.timeStamp() );
			newMessage->setHtmlBody( message.html().toString() );
			newMessage->setSubject( message.subject() );
			newMessage->setDirection( Kopete::Message::Inbound );
			newMessage->setRequestedPlugin( viewPlugin );
		}
		else if ( !body.isEmpty () )
		{
			qCDebug(JABBER_PROTOCOL_LOG) << "Received a plain text message";
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
		msg.setHtmlBody( QStringLiteral ( "<a href=\"%1\">%2</a>" ).arg ( xurl.url(), description ) );
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

	qCDebug(JABBER_PROTOCOL_LOG) << "Cached vCard data for " << contactId () << " from " << cacheDate.toString ();

	if ( !mVCardUpdateInProgress && ( cacheDate.addDays ( 1 ) < QDateTime::currentDateTime () ) )
	{
		qCDebug(JABBER_PROTOCOL_LOG) << "Scheduling update.";

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

	qCDebug(JABBER_PROTOCOL_LOG) << "Requesting vCard for " << contactId () << " from update timer.";

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
		qCDebug(JABBER_PROTOCOL_LOG) << "Scheduling request for last activity for " << mRosterItem.jid().bare ();

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
		qCDebug(JABBER_PROTOCOL_LOG) << "Requesting last activity from timer for " << mRosterItem.jid().bare ();

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

	vCard.setVersion(QStringLiteral("3.0"));
	vCard.setProdId(QStringLiteral("Kopete"));

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
		QString newLocation( QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + "jabberphotos/"+ QUrl(photoPath).fileName().toLower() ) ;
	
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
		QString newLocation( QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + "jabberphotos/"+ QUrl(photoPath).fileName().toLower() ) ;
	
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
		QString newLocation( QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + "jabberphotos/"+ QUrl(photoPath).fileName().toLower() ) ;

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
	qCDebug(JABBER_PROTOCOL_LOG) << "Message manager deleted, collecting the pieces...";

	JabberChatSession *manager = static_cast<JabberChatSession *>(sender);

	mManagers.removeAll ( manager );

}

JabberChatSession *JabberContact::manager ( Kopete::ContactPtrList chatMembers, Kopete::Contact::CanCreateFlags canCreate )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "called, canCreate: " << canCreate;

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

		qCDebug(JABBER_PROTOCOL_LOG) << "No manager found, creating a new one with resource '" << jid.resource () << "'";

		manager = new JabberChatSession ( protocol(), static_cast<JabberBaseContact *>(account()->myself()), chatMembers, jid.resource () );
		connect ( manager, SIGNAL (destroyed(QObject*)), this, SLOT (slotChatSessionDeleted(QObject*)) );
		mManagers.append ( manager );
	}

	return manager;

}

Kopete::ChatSession *JabberContact::manager ( Kopete::Contact::CanCreateFlags canCreate )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "called, canCreate: " << canCreate;

	Kopete::ContactPtrList chatMembers;
	chatMembers.append ( this );

	return manager ( chatMembers, canCreate );

}

JabberChatSession *JabberContact::manager ( const QString &resource, Kopete::Contact::CanCreateFlags canCreate )
{
	qCDebug(JABBER_PROTOCOL_LOG) << "called, canCreate: " << canCreate << ", Resource: '" << resource << "'";

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
				qCDebug(JABBER_PROTOCOL_LOG) << "Found an existing message manager for this resource.";
				return mManager;
			}
		}

		qCDebug(JABBER_PROTOCOL_LOG) << "No manager found for this resource, creating a new one.";

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

	qCDebug(JABBER_PROTOCOL_LOG) << "Resource is empty, grabbing first available manager.";

	/*
	 * The resource is empty, so just return first available manager.
	 */
	return dynamic_cast<JabberChatSession *>( manager ( canCreate ) );

}

void JabberContact::deleteContact ()
{
	qDebug (JABBER_PROTOCOL_LOG) << "Removing user " << contactId ();

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
						KStandardGuiItem::del (), KGuiItem( i18n("Keep") ),KStandardGuiItem::cancel(), QStringLiteral("JabberRemoveAuthorizationOnDelete") );
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
		sendSubscription(QStringLiteral("unsubscribe"));

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
	
	qCDebug(JABBER_PROTOCOL_LOG) << contactId () /*<< " - " <<kdBacktrace()*/;

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

	qCDebug(JABBER_PROTOCOL_LOG) << "Synchronizing contact " << contactId ();

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
		qCDebug(JABBER_PROTOCOL_LOG) << "contact has not changed,  abort sync";
		return;
	}

	XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( account()->client()->rootTask () );

	rosterTask->set ( mRosterItem.jid (), metaContact()->displayName (), mRosterItem.groups () );
	rosterTask->go (true);

}

void JabberContact::sendFile ( const QUrl &sourceURL, const QString &/*fileName*/, uint /*fileSize*/ )
{
	QString filePath;

	// if the file location is null, then get it from a file open dialog
	if ( !sourceURL.isValid () )
		filePath = QFileDialog::getOpenFileName(0L, i18n ( "Kopete File Transfer" ) ,  QString(), QStringLiteral("*"));
	else
		filePath = sourceURL.adjusted(QUrl::StripTrailingSlash).path();

	QFile file ( filePath );

	if ( file.exists () )
	{
		// send the file
		new JabberFileTransfer ( account (), this, filePath );
	}

}

void JabberContact::slotSendAuth ()
{

	qDebug (JABBER_PROTOCOL_LOG) << "(Re)send auth " << contactId ();

	sendSubscription (QStringLiteral("subscribed"));

}

void JabberContact::slotRequestAuth ()
{

	qDebug (JABBER_PROTOCOL_LOG) << "(Re)request auth " << contactId ();

	sendSubscription (QStringLiteral("subscribe"));

}

void JabberContact::slotRemoveAuth ()
{

	qDebug (JABBER_PROTOCOL_LOG) << "Remove auth " << contactId ();

	sendSubscription (QStringLiteral("unsubscribed"));

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
	int currentItem = QString ( static_cast<const QAction *>( sender() )->objectName () ).toUInt ();

	/*
	 * Warn the user if there is already an active chat window.
	 * The resource selection will only apply for newly opened
	 * windows.
	 */
	if ( manager ( Kopete::Contact::CannotCreate ) != 0 )
	{
		KMessageBox::information ( Kopete::UI::Global::mainWidget (),
										i18n ("You have preselected a resource for contact %1, "
										"but you still have open chat windows for this contact. "
										"The preselected resource will only apply to newly opened "
										"chat windows.",  contactId () ),
										i18n ("Jabber Resource Selector") );
	}

	if (currentItem == 0)
	{
		qDebug (JABBER_PROTOCOL_LOG) << "Removing active resource, trusting bestResource().";

		account()->resourcePool()->removeLock ( rosterItem().jid() );
	}
	else
	{
		// use iconText() instead of text(), because we need stripped value without '&'
		QString selectedResource = static_cast<const QAction *>(sender())->iconText();

		qDebug (JABBER_PROTOCOL_LOG) << "Moving to resource " << selectedResource;

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
	status.setShow(QLatin1String(""));

	sendPresence ( status );

}

void JabberContact::slotStatusChatty ()
{

	XMPP::Status status;
	status.setShow (QStringLiteral("chat"));

	sendPresence ( status );

}

void JabberContact::slotStatusAway ()
{

	XMPP::Status status;
	status.setShow (QStringLiteral("away"));

	sendPresence ( status );

}

void JabberContact::slotStatusXA ()
{

	XMPP::Status status;
	status.setShow (QStringLiteral("xa"));

	sendPresence ( status );

}

void JabberContact::slotStatusDND ()
{

	XMPP::Status status;
	status.setShow (QStringLiteral("dnd"));

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
			if(ident.category == QLatin1String("gateway"))
			{
				is_transport=true;
				tr_type=ident.type;
				//name=ident.name;
		
				break;  //(we currently only support gateway)
			}
			else if (ident.category == QLatin1String("service"))
			{
				//The ApaSMSAgent is reporting itself as service (instead of gateway) which is broken.
				//we anyway support it.  See bug  127811
				if(ident.type == QLatin1String("sms"))
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
		
		qCDebug(JABBER_PROTOCOL_LOG) << ri.jid().full() << " is not a contact but a gateway   - " << this;
		
		if( Kopete::AccountManager::self()->findAccount( protocol()->pluginId() , account()->accountId() + '/' + ri.jid().bare() ) )
		{
			qCDebug(JABBER_PROTOCOL_LOG) << "oops, transport already exists, abort operation ";
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
	qCDebug(JABBER_PROTOCOL_LOG) << "Start a Jingle Session";

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

