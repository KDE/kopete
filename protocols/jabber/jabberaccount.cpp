
/***************************************************************************
                   jabberaccount.cpp  -  core Jabber account class
                             -------------------
    begin                : Sat Mär 8 2003
    copyright            : (C) 2003 by Till Gerken <till@tantalo.net>
							Based on JabberProtocol by Daniel Stone <dstone@kde.org>
							and Till Gerken <till@tantalo.net>.

			   Kopete (C) 2001-2003 Kopete developers
			   <kopete-devel@kde.org>.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "jabberaccount.h"

#include <qapplication.h>
#include <qcursor.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qstring.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <klineeditdlg.h>
#include <kapplication.h>
#include <kaboutdata.h>

#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetemessagemanager.h"
#include "kopeteaway.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopeteplugin.h"
#include "kopeteuiglobal.h"
#include "kopetegroup.h"
#include "addcontactpage.h"
#include "dlgjabbersendraw.h"
#include "dlgjabberservices.h"
#include "dlgjabberchatjoin.h"
#include "jabberaddcontactpage.h"
#include "jabbergroupchat.h"

#include "xmpp_jid.h"
#include "xmpp_client.h"
#include "xmpp_stream.h"
#include "xmpp_tasks.h"
#include "xmpp_types.h"
#include "xmpp_vcard.h"

#include <sys/utsname.h>


JabberAccount::JabberAccount (JabberProtocol * parent, const QString & accountId, const char *name):KopeteAccount (parent, accountId, name)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Instantiating new account " << accountId << endl;

	mProtocol = parent;

	setMyself( new JabberContact (accountId, accountId.section('@', 0, 0), QStringList (), this, 0L) );

	jabberClient = 0L;
	registerFlag = 0;

	awayDialog = new JabberAwayDialog(this);

	initialPresence = protocol()->JabberKOSOnline;

	/*
	 * Load the SSL support library. We need to that here as
	 * otherwise, libpsi won't recognize SSL support. (loading
	 * SSL just before doing setSSLEnabled() will fail)
	 * The string list is just a dummy, QSSL is linked statically.
	 */
	QStringList dirs = "/usr/lib";
	Jabber::Stream::loadSSL (dirs);

}

JabberAccount::~JabberAccount ()
{
	disconnect ();

	Jabber::Stream::unloadSSL ();

	if (jabberClient)
	{
		delete jabberClient;

		jabberClient = 0L;
	}

	delete awayDialog;
}

KActionMenu *JabberAccount::actionMenu ()
{
	KActionMenu *m_actionMenu = new KActionMenu( accountId(), myself()->onlineStatus().iconFor(this),  this );

	m_actionMenu->popupMenu()->insertTitle(
		myself()->onlineStatus().iconFor(myself()),
		i18n("%2 <%1>")
#if QT_VERSION < 0x030200
			.arg(accountId()).arg(myself()->displayName()));
#else
			.arg(accountId(), myself()->displayName()));
#endif

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSOnline.caption(),
		mProtocol->JabberKOSOnline.iconFor(this), 0, this, SLOT (slotGoOnline ()), this,
		"actionJabberConnect"));

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSChatty.caption(),
		mProtocol->JabberKOSChatty.iconFor(this), 0, this, SLOT (slotGoChatty ()), this,
		"actionJabberChatty"));

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSAway.caption(),
		mProtocol->JabberKOSAway.iconFor(this), 0, this, SLOT (slotGoAway ()), this,
		"actionJabberAway"));

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSXA.caption(),
		mProtocol->JabberKOSXA.iconFor(this), 0, this, SLOT (slotGoXA ()), this,
		"actionJabberXA"));

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSDND.caption(),
		mProtocol->JabberKOSDND.iconFor(this), 0, this, SLOT (slotGoDND ()), this,
		"actionJabberDND"));

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSInvisible.caption(),
		mProtocol->JabberKOSInvisible.iconFor(this), 0, this, SLOT (slotGoInvisible ()), this,
		"actionJabberInvisible"));

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSOffline.caption(),
		mProtocol->JabberKOSOffline.iconFor(this), 0, this,SLOT (slotGoOffline ()), this,
		"actionJabberDisconnect"));

	m_actionMenu->popupMenu()->insertSeparator();
	m_actionMenu->insert(new KAction (i18n ("Join Groupchat..."), "jabber_group", 0,
		this, SLOT (slotJoinNewChat ()), this, "actionJoinChat"));
	m_actionMenu->popupMenu()->insertSeparator();
	m_actionMenu->insert(new KAction (i18n ("Services..."), "jabber_serv_on", 0,
		this, SLOT (slotGetServices ()), this, "actionJabberServices"));
	m_actionMenu->insert(new KAction (i18n ("Send Raw Packet to Server..."), "mail_new", 0,
		this, SLOT (slotSendRaw ()), this, "actionJabberSendRaw"));
	m_actionMenu->insert(new KAction (i18n ("Edit User Info..."), "identity", 0,
		this, SLOT (slotEditVCard ()), this, "actionEditVCard"));

	return m_actionMenu;
}

/*
 *  Add a contact to Meta Contact
 */
bool JabberAccount::addContactToMetaContact (const QString & contactId, const QString & displayName, KopeteMetaContact * metaContact)
{

	/* collect all group names */
	QStringList groupNames;
	QPtrList<KopeteGroup> groupList = metaContact->groups();
	for(KopeteGroup *group = groupList.first(); group; group = groupList.next())
		groupNames += group->displayName();


	JabberContact *jc = createContact(contactId, displayName, groupNames, metaContact);

	return (jc != 0);

}

void JabberAccount::errorConnectFirst ()
{
	KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget (), KMessageBox::Error,  i18n ("Please connect first."), i18n ("Error"));
}


void JabberAccount::connect ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called" << endl;

	/* Don't do anything if we are already connected. */
	if (isConnected ())
		return;

	/* This is dirty but has to be done: if a previous connection attempt
	 * failed, psi doesn't handle recovering too well. we are not allowed to
	 * call close in the slotConnected() slot since it causes a crash, so we
	 * have to delete the psi backend altogether here for safety if it still
	 * exists. */
	if (jabberClient)
	{
		jabberClient->close ();
		delete jabberClient;

		jabberClient = 0L;
	}

	jabberClient = new Jabber::Client (this);

	/* This should only be done here to connect the signals, otherwise it is a
	 * bad idea.
	 */
	{
		using namespace Jabber;
		QObject::connect (jabberClient, SIGNAL (handshaken ()), this, SLOT (slotHandshaken ()));
		QObject::connect (jabberClient, SIGNAL (authFinished (bool, int, const QString &)), this, SLOT (slotConnected (bool, int, const QString &)));
		QObject::connect (jabberClient, SIGNAL (closeFinished ()), this, SLOT (slotDisconnected ()));
		QObject::connect (jabberClient, SIGNAL (subscription (const Jid &, const QString &)), this, SLOT (slotSubscription (const Jid &, const QString &)));
		QObject::connect (jabberClient, SIGNAL (rosterItemAdded (const RosterItem &)), this, SLOT (slotNewContact (const RosterItem &)));
		QObject::connect (jabberClient, SIGNAL (rosterItemUpdated (const RosterItem &)), this, SLOT (slotContactUpdated (const RosterItem &)));
		QObject::connect (jabberClient, SIGNAL (rosterItemRemoved (const RosterItem &)), this, SLOT (slotContactDeleted (const RosterItem &)));
		QObject::connect (jabberClient, SIGNAL (resourceAvailable (const Jid &, const Resource &)), this,
						  SLOT (slotResourceAvailable (const Jid &, const Resource &)));
		QObject::connect (jabberClient, SIGNAL (resourceUnavailable (const Jid &, const Resource &)), this,
						  SLOT (slotResourceUnavailable (const Jid &, const Resource &)));
		QObject::connect (jabberClient, SIGNAL (messageReceived (const Message &)), this, SLOT (slotReceivedMessage (const Message &)));
		QObject::connect (jabberClient, SIGNAL (groupChatJoined (const Jid &)), this, SLOT (slotGroupChatJoined (const Jid &)));
		QObject::connect (jabberClient, SIGNAL (groupChatLeft (const Jid &)), this, SLOT (slotGroupChatLeft (const Jid &)));
		QObject::connect (jabberClient, SIGNAL (groupChatPresence (const Jid &, const Status &)), this, SLOT (slotGroupChatPresence (const Jid &, const Status &)));
		QObject::connect (jabberClient, SIGNAL (groupChatError (const Jid &, int, const QString &)), this,
						  SLOT (slotGroupChatError (const Jid &, int, const QString &)));
		QObject::connect (jabberClient, SIGNAL (sslCertReady (const QSSLCert &)), jabberClient, SLOT (continueAfterCert ()));
		QObject::connect (jabberClient, SIGNAL (error (const StreamError &)), this, SLOT (slotError (const StreamError &)));
		QObject::connect (jabberClient, SIGNAL (debugText (const QString &)), this, SLOT (slotPsiDebug (const QString &)));
	}

	struct utsname utsBuf;

	uname (&utsBuf);

	jabberClient->setClientName ("Kopete");
	jabberClient->setClientVersion (kapp->aboutData ()->version ());
	jabberClient->setOSName (QString ("%1 %2").arg (utsBuf.sysname, 1).arg (utsBuf.release, 2));

	/* Check if we are capable of using SSL if requested. */
	if (pluginData (protocol (), "UseSSL") == "true")
	{
		bool sslPossible = jabberClient->setSSLEnabled (true);

		if (!sslPossible)
		{
			KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget (), KMessageBox::Error,
								i18n ("SSL is not supported. This is most likely because the QSSL library could not be found."), i18n ("SSL Error"));
			return;
		}
	}

	/* Parse proxy settings. */
	QString proxyTypeStr = pluginData (protocol (), "ProxyType");
	int proxyType = Jabber::StreamProxy::None;

	if (proxyTypeStr == QString ("HTTPS"))
		proxyType = Jabber::StreamProxy::HTTPS;
	else
	{
		if (proxyTypeStr == QString ("SOCKS4"))
			proxyType = Jabber::StreamProxy::SOCKS4;
		else if (proxyTypeStr == QString ("SOCKS5"))
			proxyType = Jabber::StreamProxy::SOCKS5;
	}

	Jabber::StreamProxy proxy (proxyType, pluginData (protocol (), "ProxyName"), pluginData (protocol (), "ProxyPort").toInt ());

	proxy.setUseAuth (pluginData (protocol (), "ProxyAuth") == QString::fromLatin1 ("true"));
	proxy.setUser (pluginData (protocol (), "ProxyUser"));
	proxy.setPass (pluginData (protocol (), "ProxyPass"));

	jabberClient->setProxy (proxy);

	QString jidDomain = accountId ().section ("@", 1);

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Connecting to Jabber server " << server() << ":" << port() << " with jidDomain " << jidDomain << endl;

	setPresence(protocol()->JabberKOSConnecting, "");

	jabberClient->connectToHost (server(), port(), jidDomain);

}

void JabberAccount::slotPsiDebug (const QString & _msg)
{
	QString msg = _msg;

	msg = msg.replace( QRegExp( "<password>[^<]*</password>\n" ), "<password>[Filtered]</password>\n" );
	msg = msg.replace( QRegExp( "<digest>[^<]*</digest>\n" ), "<digest>[Filtered]</digest>\n" );

	kdDebug (JABBER_DEBUG_PROTOCOL) << k_funcinfo << "Psi: " << msg << endl;

}

void JabberAccount::slotHandshaken ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Performing login..." << endl;

	if (registerFlag)
	{
		Jabber::JT_Register * task = new Jabber::JT_Register (jabberClient->rootTask ());
		QObject::connect (task, SIGNAL (finished ()), this, SLOT (slotRegisterUserDone ()));
		task->reg (accountId().section("@", 0, 0), password());
		task->go (true);
	}
	else
	{
		if (pluginData (protocol (), "AuthType") == QString ("digest"))
			jabberClient->authDigest (accountId().section("@", 0, 0), password(), resource());
		else
			jabberClient->authPlain (accountId().section("@", 0, 0), password(), resource());

	}

}

void JabberAccount::slotConnected (bool success, int statusCode, const QString & statusString)
{
	if (success)
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Connected to Jabber server." << endl;

		/* Request roster. */
		jabberClient->rosterRequest ();

		/* Since we are online now, set initial presence. Don't do this
		 * before the roster request or we will receive presence
		 * information before we have updated our roster with actual
		 * contacts from the server! (libpsi won't forward presence
		 * information in that case either). */
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Setting Presence." << endl;
		setPresence (initialPresence, static_cast<JabberContact *>( myself() )->reason ());

		/* Initiate anti-idle timer (will be triggered every 120
		 * seconds). */
		jabberClient->setNoopTime (120000);
	}
	else
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Connection failed! Status: " << statusCode << ", " << statusString << endl;

		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget (), KMessageBox::Error, i18n ("Connection failed with reason \"%1\".").arg (statusString, 1), i18n ("Connection Failed"));
		disconnect();
	}

}

void JabberAccount::disconnect ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] disconnect() called" << endl;

	if (isConnected ())
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Still connected, closing connection..." << endl;
		/* Tell backend class to disconnect. */
		jabberClient->close ();
	}

	/* FIXME:
	 * We should delete the Jabber::Client instance here,
	 * but active timers in psi prevent us from doing so.
	 * (in a failed connection attempt, these timers will
	 * try to access an already deleted object).
	 * Instead, the instance will lurk until the next
	 * connection attempt.
	 */
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Disconnected." << endl;

}

void JabberAccount::slotConnect ()
{
	connect ();
}

void JabberAccount::slotDisconnect ()
{
	disconnect ();
}

void JabberAccount::slotDisconnected ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Disconnected from Jabber server." << endl;

	/* FIXME:
	 * We should delete the Jabber::Client instance here,
	 * but timers etc prevent us from doing so. (Psi does
	 * not like to be deleted from a slot).
	 */

	/* It seems that we don't get offline notifications when going offline
	 * with the protocol, so update all contacts manually. */
	for (QDictIterator < KopeteContact > it (contacts ()); it.current (); ++it)
		static_cast < JabberContact * >(*it)->slotUpdatePresence (protocol()->JabberKOSOffline, "disconnected");
}

void JabberAccount::slotError (const Jabber::StreamError & error)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Error in stream signalled, disconnecting." << endl;

	/* Determine type of error. */
	switch (error.type ())
	{
	case Jabber::StreamError::DNS:
		KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (), KMessageBox::Error,
							i18n
							("Connection to the Jabber server %1 for account %2 failed due to a DNS error (%1); check you typed the server name correctly.").
							arg (server(), 1).arg (accountId(), 2).arg (error.details (), 3), i18n ("Error Connecting to Jabber Server"));
		break;

	case Jabber::StreamError::Refused:
		KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (), KMessageBox::Error,
							i18n
							("The connection was refused when attempting to contact the server %1 for the account %2; check both the server name and the port number.").
							arg (server(), 1).arg (accountId(), 2), i18n ("Error Connecting to Jabber Server"));
		break;

	case Jabber::StreamError::Timeout:
		KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (), KMessageBox::Error, 
							i18n
							("The connection to the Jabber server %1 for the account %2 timed out.").
							arg (server(), 1).arg (accountId(), 2), i18n ("Error Connecting to Jabber Server"));
		break;

	case Jabber::StreamError::Socket:
		KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (), KMessageBox::Error,
							i18n
							("There was a socket error (%1); your connection to the Jabber server %2 for account %3 has been lost.").
							arg (error.details (), 1).arg (server(), 2).arg (accountId(), 3), i18n ("Error Connecting to Jabber Server"));
		break;

	case Jabber::StreamError::Disconnected:
		KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (), KMessageBox::Error,
							i18n
							("The remote server %1 closed the connection for account %2, without specifying any error. This usually means that the server is broken.").
							arg (server(), 1).arg (accountId(), 2), i18n ("Error Connecting to Jabber Server"));
		break;

	case Jabber::StreamError::Handshake:
		KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (), KMessageBox::Error,
							i18n
							("Connection to the Jabber server failed due to the handshake failing (%1); check that you typed your Jabber ID and password. Note that the Jabber ID now needs to be done in full user@domain form, not just the username.").
							arg (error.details (), 1), i18n ("Error Connecting to Jabber Server"));
		break;

	case Jabber::StreamError::SSL:
		KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (), KMessageBox::Error,
							i18n
							("Connection to the Jabber server failed due to a SSL error (%1); this usually means that the server's SSL implementation is broken.").
							arg (error.details (), 1), i18n ("Error Connecting to Jabber Server"));
		break;

	case Jabber::StreamError::Proxy:
		KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (), KMessageBox::Error,
							i18n
							("Connection to the Jabber server failed due to a proxy error (%1).").
							arg (error.details (), 1), i18n ("Error Connecting to Jabber Server"));
		break;

	case Jabber::StreamError::Unknown:
	default:
		KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (), KMessageBox::Error,
							i18n
							("An unknown error was encountered (%1); please report this error to kopete-devel@kde.org, along with what you were doing at the time.").
							arg (error.details (), 1), i18n ("Error Connecting to Jabber Server"));
		break;
	}

	disconnect ();

	// manually force the slot to be called since in case of an error,
	// libpsi will most likely be confused and not emit signals anymore
	slotDisconnected();

}

/* Set presence (usually called by dialog widget). */
void JabberAccount::setPresence (const KopeteOnlineStatus & status, const QString & reason, int priority)
{

	/*
	 * If we are in the process of connecting, only update our local presence
	 * and don't send anything across the wire.
	 */
	if(status == protocol()->JabberKOSConnecting)
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Setting new presence locally (-> connecting)." << endl;

		static_cast<JabberContact *>( myself() )->slotUpdatePresence (status, reason);
	}
	else
	{
		/*
		 * If we are already connected and changing our presence or if we are connecting
		 * and set our initial presence, send new presence packet to the server.
		 * Sorry for the ugly if() below but the requirement for certain timings to send out presence
		 * packets and the silly implementation of KopeteAccount::isOnline() leave no other choice.
		 */
		if (isConnected())
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Sending new presence to the server." << endl;

			Jabber::Status presence;

			presence.setPriority (priority);
			presence.setStatus (reason);
			presence.setIsAvailable (true);

			if (status == protocol()->JabberKOSOnline)
				presence.setShow ("");
			else if (status == protocol()->JabberKOSChatty)
				presence.setShow ("chat");
			else if (status == protocol()->JabberKOSAway)
				presence.setShow ("away");
			else if (status == protocol()->JabberKOSXA)
				presence.setShow ("xa");
			else if (status == protocol()->JabberKOSDND)
				presence.setShow ("dnd");
			else if (status == protocol()->JabberKOSInvisible)
				presence.setIsInvisible (true);
			else
			{
				kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Unknown presence status, " << "ignoring (status == " << status.description () << ")" << endl;
				return;
			}

			kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Updating presence to show(" << presence.show () << "), status(" << presence.status () << "), with reason \"" << reason << endl;

			static_cast<JabberContact *>( myself() )->slotUpdatePresence (status, reason);

			Jabber::JT_Presence * task = new Jabber::JT_Presence (jabberClient->rootTask ());

			task->pres (presence);
			task->go (true);
		}
		else
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "We were not connected, presence update aborted." << endl;
		}
	}

}

void JabberAccount::setAway (bool away, const QString & reason)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Setting away mode: " << away << endl;

	if(away)
		setPresence (protocol()->JabberKOSAway, reason);
	else
		setPresence (protocol()->JabberKOSOnline, reason);

}

void JabberAccount::setAvailable (void)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Coming back from away mode." << endl;
	slotGoOnline ();
}

void JabberAccount::slotGoOnline ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	if (!isConnected ())
	{
		/* We are not connected yet, so connect now. */
		initialPresence = protocol()->JabberKOSOnline;
		connect ();
	}
	else
	{
		setPresence (protocol()->JabberKOSOnline, "");
	}

}

void JabberAccount::slotGoOffline ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	disconnect ();
}

void JabberAccount::slotGoChatty ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	if (!isConnected ())
	{
		/* We are not connected yet, so connect now. */
		initialPresence = protocol()->JabberKOSChatty;
		connect ();
	}
	else
	{
		setPresence (protocol()->JabberKOSChatty, "");
	}

}

void JabberAccount::slotGoAway ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	if (!isConnected ())
	{
		/* We are not connected yet, so connect now. */
		initialPresence = protocol()->JabberKOSAway;
		connect ();
	}
	else
	{
		awayDialog->show(JabberProtocol::JabberAway);
	}

}

void JabberAccount::slotGoXA ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	if (!isConnected ())
	{
		/* We are not connected yet, so connect now. */
		initialPresence = protocol()->JabberKOSXA;
		connect ();
	}
	else
	{
		awayDialog->show(JabberProtocol::JabberXA);
	}

}

void JabberAccount::slotGoDND ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	if (!isConnected ())
	{
		/* We are not connected yet, so connect now. */
		initialPresence = protocol()->JabberKOSDND;
		connect ();
	}
	else
	{
		awayDialog->show(JabberProtocol::JabberDND);
	}

}

void JabberAccount::slotGoInvisible ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	if (!isConnected ())
	{
		/* We are not connected yet, so connect now. */
		initialPresence = protocol()->JabberKOSInvisible;
		connect ();
	}
	else
	{
		setPresence (protocol()->JabberKOSInvisible, "");
	}

}

void JabberAccount::slotSendRaw ()
{
	/* Check if we're connected. */
	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	new dlgJabberSendRaw (jabberClient, Kopete::UI::Global::mainWidget());

}

void JabberAccount::subscribe (const Jabber::Jid & jid)
{

	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	Jabber::JT_Presence * task = new Jabber::JT_Presence (jabberClient->rootTask ());

	task->sub (jid, "subscribe");
	task->go (true);
}

void JabberAccount::subscribed (const Jabber::Jid & jid)
{
	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	Jabber::JT_Presence * task = new Jabber::JT_Presence (jabberClient->rootTask ());

	task->sub (jid, "subscribed");
	task->go (true);
}

void JabberAccount::unsubscribed (const Jabber::Jid & jid)
{
	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	Jabber::JT_Presence * task = new Jabber::JT_Presence (jabberClient->rootTask ());

	task->sub (jid, "unsubscribed");
	task->go (true);
}

void JabberAccount::sendPresenceToNode (const KopeteOnlineStatus & pres, const QString & userID)
{

	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	Jabber::JT_Presence * task = new Jabber::JT_Presence (jabberClient->rootTask ());

	Jabber::Jid jid (userID);
	Jabber::Status status;

	if (pres == protocol()->JabberKOSOnline)
		status.setShow ("");
	else if (pres == protocol()->JabberKOSChatty)
		status.setShow ("chat");
	else if (pres == protocol()->JabberKOSAway)
		status.setShow ("away");
	else if (pres == protocol()->JabberKOSXA)
		status.setShow ("xa");
	else if (pres == protocol()->JabberKOSDND)
		status.setShow ("dnd");
	else if (pres == protocol()->JabberKOSInvisible)
	{
		status.setShow ("away");
		status.setIsInvisible (true);
	}
	else
		status.setShow ("away");

	task->pres (jid, status);
	task->go (true);
}

JabberContact *JabberAccount::createContact (const QString & jid, const QString & alias, const QStringList & groups, KopeteMetaContact * metaContact)
{

	JabberContact *jc = new JabberContact (jid, alias, groups, this, metaContact);

	return jc;

}


void JabberAccount::createAddContact (KopeteMetaContact * mc, const Jabber::RosterItem & item)
{

	if (!mc)
	{
		/*
		 * If no metacontact has been given, try to locate an existing one
		 * that contains a contact with the same ID that we are to create.
		 */
		mc = KopeteContactList::contactList ()->findContact (protocol ()->pluginId (), accountId (), item.jid ().userHost ().lower());

		if (mc)
		{
			/*
			 * A metacontact exists that does contain a contact with the same ID
			 */
			JabberContact *jc = (JabberContact *) mc->findContact (protocol ()->pluginId (),
																   accountId (),
																   item.jid ().userHost ().lower());

			if (jc)
			{
				/*
				 * Since the subcontact exists already, we don't recreate it but
				 * merely update its data according to our parameters.
				 */
				kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Contact " << item.jid ().userHost () << " already exists, updating" << endl;
				jc->slotUpdateContact (item);
				return;
			}
			else
			{
				/*
				 * If this code is reached, something is severely broken:
				 * A subcontact of a metacontact exists but we are unable to
				 * retrieve it's pointer.
				 */
				kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "****Warning**** : " << item.jid ().userHost () << " already exists, and can be found" << endl;
			}
		}
	}

	/*
	 * If we got here and mc is still NULL, the contact is not
	 * in the contact list yet and we need to create a new metacontact
	 * for it.
	 */
	bool isContactInList = true;
	if (!mc)
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Adding contact " << item.jid ().userHost () << " ..." << endl;

		isContactInList = false;

		mc = new KopeteMetaContact ();
		QStringList groups = item.groups ();

		for (QStringList::Iterator it = groups.begin (); it != groups.end (); ++it)
			mc->addToGroup (KopeteContactList::contactList ()->getGroup (*it));
	}

	/*
	 * At this point, we either found the metacontact or created a new
	 * one. The only thing left to do is to create a new Jabber contact
	 * inside it.
	 */
	QString contactName;

	if (item.name ().isNull () || item.name ().isEmpty ())
		contactName = item.jid ().userHost ();
	else
		contactName = item.name ();

	createContact (item.jid().userHost(), contactName, item.groups(), mc);

	if (!isContactInList)
		KopeteContactList::contactList ()->addMetaContact (mc);

}

void JabberAccount::slotSubscription (const Jabber::Jid & jid, const QString & type)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] slotSubscription(" << jid.userHost () << ", " << type << ");" << endl;

	if (type == "subscribe")
	{
		/* A new user wants to subscribe. */
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] slotSubscription(): " << jid.userHost () << " is asking for authorization to subscribe." << endl;

		switch (KMessageBox::questionYesNoCancel (Kopete::UI::Global::mainWidget (),
												  i18n
												  ("The Jabber user %1 wants to add you to their "
												   "contact list; do you want to authorize them? "
												   "Selecting Cancel will ignore the request.").
												  arg (jid.userHost (), 1), i18n ("Authorize Jabber User?"), i18n ("Authorize"), i18n ("Deny")))
		{
			KopeteMetaContact *mc;

		case KMessageBox::Yes:
			/* Authorize user. */
			subscribed (jid);

			/* Is the user already in our contact list? */
			mc = KopeteContactList::contactList ()->findContact (protocol ()->pluginId (), accountId (), jid.userHost ());

			/* If it is not, ask the user if he wants to subscribe in return. */
			if ((!mc || mc->isTemporary()) && (KMessageBox::questionYesNo (Kopete::UI::Global::mainWidget (),
													i18n
													("Do you want to add %1 to your contact "
													 "list in return?").arg (jid.userHost (), 1), i18n ("Add Jabber User?")) == KMessageBox::Yes))
				/* Subscribe to user's presence. */
				subscribe (jid);
			break;

		case KMessageBox::No:
			/* Reject subscription. */
				unsubscribed (jid);

			break;

		case KMessageBox::Cancel:
			/* Leave the user in the dark. */
			break;
		}

	}
	else if (type == "unsubscribed")
	{
		/* Someone else removed us from their roster. */
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] " << jid.userHost () << " deleted auth!" << endl;

		Jabber::JT_Roster * task = new Jabber::JT_Roster (jabberClient->rootTask ());
		switch (KMessageBox::warningYesNo (Kopete::UI::Global::mainWidget(),
								  i18n
								  ("The Jabber user %1 removed %2's subscription to them."
								   "This account will no longer be able to view their online/offline status."
								   "\nDo you want to delete the contact?").
								  arg (jid.userHost (), 1).arg (accountId(), 2), i18n ("Notification")))
		{

		case KMessageBox::Yes:
			task->remove (jid);
			task->go (true);
			break;

		default:
			/* We want to leave the contact in our contact list, so do nothing. */
			break;

		}
	}
}

void JabberAccount::slotNewContact (const Jabber::RosterItem & item)
{

	/**
	 * Subscription types are: Both, From, To, Remove, None.
	 * Both:   Both sides have authed each other, each side
	 *         can see each other's presence
	 * From:   The other side can see us.
	 * To:     We can see the other side. (implies we are
	 *         authed)
	 * Remove: Other side revoked our subscription request.
	 *         Not to be handled here.
	 * None:   No subscription.
	 *
	 * Regardless of the subscription type, we have to add
	 * a roster item here. FIXME: To be done is uniform
	 * check of the ask() value to see if we are waiting
	 * for authorization ("subscribe")
	 */

	QString debugStr = "[JabberAccount] New Contact " + item.jid ().userHost () + " (Subscription::";

	switch (item.subscription ().type ())
	{
	case Jabber::Subscription::Both:	// both sides can see the contact
		debugStr += "Both | <->";
		break;

	case Jabber::Subscription::From:	// he can see us
		debugStr += "From | <--";
		break;

	case Jabber::Subscription::To:	// we can see him
		debugStr += "To | -->";
		break;

	case Jabber::Subscription::None:	// waiting for authorization
		debugStr += "None | ---";
		break;
	}

	debugStr += ") " + item.ask ();

	kdDebug (JABBER_DEBUG_GLOBAL) << debugStr << endl;

	/* Add the contact to the GUI. */
	createAddContact (0L, item);
}

void JabberAccount::slotContactDeleted (const Jabber::RosterItem & item)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Deleting contact " << item.jid ().userHost () << endl;

	if (!contacts ()[item.jid().userHost().lower()])
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberProtocl] WARNING: slotContactDeleted() " << "was asked to delete a non-existing contact." << endl;
		return;
	}

	JabberContact *jc = static_cast < JabberContact * >(contacts ()[item.jid().userHost().lower()]);

	/* This will also cause the contact to disappear from the metacontact. */
	delete jc;
}

void JabberAccount::slotContactUpdated (const Jabber::RosterItem & item)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Status update for " << item.jid ().userHost () << endl;

	/* Sanity check. */
	if (!contacts ()[item.jid().userHost().lower()])
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] WARNING: slotContactUpdated() " << "was requested to update a non-existing contact." << endl;
		return;
	}
	// update the contact data
	static_cast < JabberContact * >(contacts ()[item.jid().userHost().lower()])->slotUpdateContact (item);
}

void JabberAccount::slotReceivedMessage (const Jabber::Message & message)
{
	QString userHost;
	JabberContact *contactFrom;

	userHost = message.from ().userHost ();
	contactFrom = static_cast < JabberContact * >(contacts ()[userHost.lower()]);

	if (userHost.isEmpty ())
	{
		/* If the sender field is empty, it is a server message.
		 *
		 * When I wrote it, this made sense, but should it be displayed
		 * in a KopeteEmailWindow now? -DS */
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] New server message for us!" << endl;

		KMessageBox::information (Kopete::UI::Global::mainWidget (), message.body (), i18n ("Jabber: Server Message"));
	}
	else
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] New message from '" << userHost << "'" << endl;

		/* See if the contact is actually in our roster. */
		if (!contactFrom)
		{
			/* So, either it's a group chat, or we're not subscribed
			 * to them. Either way. */
			kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Message received from an " << "unknown contact, creating temporary contact." << endl;

			KopeteMetaContact *metaContact = new KopeteMetaContact ();

			metaContact->setTemporary (true);

			contactFrom = createContact (userHost, userHost, QStringList (), metaContact);

			KopeteContactList::contactList ()->addMetaContact (metaContact);
		}

		/* Pass the message on to the contact. */
		contactFrom->slotReceivedMessage (message);
	}

}

void JabberAccount::slotJoinNewChat ()
{
	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	dlgJabberChatJoin *dlg = new dlgJabberChatJoin (this, Kopete::UI::Global::mainWidget ());

	dlg->show ();
	dlg->raise ();
}

void JabberAccount::slotGroupChatJoined (const Jabber::Jid & jid)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Joined group chat " << jid.full () << endl;

	/* Create new meta contact that holds the group chat contact. */
	KopeteMetaContact *mc = new KopeteMetaContact ();

	mc->setTemporary (true);

	/* The group chat object basically works like a JabberContact. */
	JabberGroupChat *groupChat = new JabberGroupChat (jid, QStringList (), this, mc);

	/* Add the group chat class to the meta contact. */
	mc->addContact (groupChat);

	KopeteContactList::contactList ()->addMetaContact (mc);
}

void JabberAccount::slotGroupChatLeft (const Jabber::Jid & jid)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Left groupchat " << jid.full () << endl;
	delete static_cast < JabberGroupChat * >(contacts ()[jid.userHost().lower()]);
}

void JabberAccount::slotGroupChatPresence (const Jabber::Jid & jid, const Jabber::Status & status)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Received groupchat presence for room " << jid.full () << endl;
	static_cast < JabberGroupChat * >(contacts ()[jid.userHost().lower()])->updatePresence (jid, status);
}

void JabberAccount::slotGroupChatError (const Jabber::Jid & jid, int error, const QString & reason)
{
	/* FIXME: Present this to the user, damnit! */
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Group chat error - room " << jid.userHost () << " had error " << error << " (" << reason << ")!" << endl;
}

void JabberAccount::slotResourceAvailable (const Jabber::Jid & jid, const Jabber::Resource & resource)
{

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "New resource available for " << jid.userHost () << endl;

	if (!contacts ()[jid.userHost().lower()])
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Trying to add a resource, but " << "couldn't find an entry for " << jid.userHost () << endl;
		return;
	}

	if(static_cast<JabberContact *>(contacts ()[jid.userHost().lower()]) == myself())
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Ignoring resource by other client for ourselves." << endl;
		return;
	}

	static_cast < JabberContact * >(contacts ()[jid.userHost().lower()])->slotResourceAvailable (jid, resource);
}

void JabberAccount::slotResourceUnavailable (const Jabber::Jid & jid, const Jabber::Resource & resource)
{

	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Resource now unavailable for " << jid.userHost () << endl;

	if (!contacts ()[jid.userHost().lower()])
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Trying to remove a resource, " << "but couldn't find an entry for " << jid.userHost () << endl;
		return;
	}

	if(static_cast<JabberContact *>(contacts ()[jid.userHost().lower()]) == myself())
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Ignoring resource by other client for ourselves." << endl;
		return;
	}

	static_cast < JabberContact * >(contacts ()[jid.userHost().lower()])->slotResourceUnavailable (jid, resource);

}

void JabberAccount::slotEditVCard ()
{
	static_cast<JabberContact *>( myself() )->slotEditVCard ();
}

void JabberAccount::registerUser ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Registering user " << accountId() << " on server " << server() << "." << endl;

	/* Save the current preferences. */
	//preferences->save();

	/* Set the flag to register an account during registration. */
	registerFlag = 1;

	/* Now connect, initiating the registration. */
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Register: Connect() " << endl;
	connect ();
}

void JabberAccount::slotRegisterUserDone ()
{
	Jabber::JT_Register * task = (Jabber::JT_Register *) sender ();

	if (task->success ())
		KMessageBox::information (Kopete::UI::Global::mainWidget (), i18n ("Account successfully registered."), i18n ("Account Registration"));
	else
	{
		KMessageBox::information (Kopete::UI::Global::mainWidget (), i18n ("Unable to create account on the server."), i18n ("Account Registration"));

	}
	disconnect ();
	registerFlag = 0;
}

bool JabberAccount::addContact( const QString &contactId, const QString &displayName,
							   KopeteMetaContact *parentContact, const KopeteAccount::AddMode mode, const QString &groupName,
							   bool isTemporary)
{
	Jabber::RosterItem item;

	item.setJid(Jabber::Jid(contactId));
	item.setName(contactId);
	item.setGroups(groupName);

	//createAddContact(parentContact, item);

	// add the new contact to our roster.
	Jabber::JT_Roster * rosterTask = new Jabber::JT_Roster(jabberClient->rootTask());

	rosterTask->set(item.jid(), item.name(), item.groups());
	rosterTask->go(true);

	// send a subscription request.
	subscribe(item.jid());

	return KopeteAccount::addContact(contactId, displayName, parentContact, mode, groupName, isTemporary);

}

void JabberAccount::removeContact (const Jabber::RosterItem & item)
{
	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	Jabber::JT_Roster * rosterTask = new Jabber::JT_Roster (jabberClient->rootTask ());

	rosterTask->remove (item.jid ());
	rosterTask->go (true);
}

const QString JabberAccount::resource () const
{

	return pluginData (protocol (), "Resource");

}

const QString JabberAccount::server () const
{

	return pluginData (protocol (), QString::fromLatin1 ("Server"));

}

const int JabberAccount::port () const
{

	return pluginData (protocol (), "Port").toInt ();

}

Jabber::Client *JabberAccount::client()
{

	return jabberClient;

}

void JabberAccount::slotGetServices ()
{
	dlgJabberServices *dialog = new dlgJabberServices (this);

	dialog->show ();
	dialog->raise ();
}

#include "jabberaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

