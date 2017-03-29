 /*
  *   jabberprotocol.cpp  -  Base class for the Kopete Jabber protocol
  *
  * Copyright (c) 2002-2003 by Till Gerken <till@tantalo.net>
  * Copyright (c) 2002 by Daniel Stone <dstone@kde.org>
  * Copyright (c) 2006      by Olivier Goffart  <ogoffart at kde.org>
  *
  *  Kopete   (c) by the Kopete developers  <kopete-devel@kde.org>
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

#include "jabberprotocol.h"
#include "jabber_protocol_debug.h"
#include <kpluginfactory.h>
#include <kconfig.h>

#include <qmap.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <QList>
#include <QPointer>

#include "im.h"
#include "xmpp.h"

#include <sys/utsname.h>

#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetechatsession.h"
#include "kopeteonlinestatusmanager.h"
#include "kopeteglobal.h"
#include "kopeteprotocol.h"
#include "kopeteplugin.h"
#include "kopeteaccountmanager.h"
#include "addcontactpage.h"
#include "kopetecommandhandler.h"
#include "kopetedeletecontacttask.h"

#include "jabbercontact.h"
#include "jabberaddcontactpage.h"
#include "jabberaccount.h"
#include "jabbereditaccountwidget.h"
#include "jabbercapabilitiesmanager.h"
#include "jabbertransport.h"
#include "dlgjabberservices.h"
#include "dlgjabberchatjoin.h"
#include "dlgregister.h"

JabberProtocol *JabberProtocol::protocolInstance = 0;

K_PLUGIN_FACTORY( JabberProtocolFactory, registerPlugin<JabberProtocol>(); )
K_EXPORT_PLUGIN( JabberProtocolFactory( "kopete_jabber" ) )

JabberProtocol::JabberProtocol (QObject * parent, const QVariantList &)
: Kopete::Protocol(parent),
	JabberKOSChatty(Kopete::OnlineStatus::Online,        100, this, JabberFreeForChat, QStringList(QStringLiteral("jabber_chatty")), i18n ("Free for Chat"), i18n ("Free for Chat"), Kopete::OnlineStatusManager::FreeForChat, Kopete::OnlineStatusManager::HasStatusMessage ),
	JabberKOSOnline(Kopete::OnlineStatus::Online,         90, this, JabberOnline, QStringList(), i18n ("Online"), i18n ("Online"), Kopete::OnlineStatusManager::Online, Kopete::OnlineStatusManager::HasStatusMessage ),
	JabberKOSAway(Kopete::OnlineStatus::Away,             80, this, JabberAway, QStringList(QStringLiteral("contact_away_overlay")), i18n ("Away"), i18n ("Away"), Kopete::OnlineStatusManager::Away, Kopete::OnlineStatusManager::HasStatusMessage),
	JabberKOSXA(Kopete::OnlineStatus::Away,               70, this, JabberXA, QStringList(QStringLiteral("contact_xa_overlay")), i18n ("Extended Away"), i18n ("Extended Away"), Kopete::OnlineStatusManager::ExtendedAway, Kopete::OnlineStatusManager::HasStatusMessage),
	JabberKOSDND(Kopete::OnlineStatus::Busy,              60, this, JabberDND, QStringList(QStringLiteral("contact_busy_overlay")), i18n ("Do not Disturb"), i18n ("Do not Disturb"), Kopete::OnlineStatusManager::Busy, Kopete::OnlineStatusManager::HasStatusMessage),
	JabberKOSOffline(Kopete::OnlineStatus::Offline,       50, this, JabberOffline, QStringList(), i18n ("Offline") ,i18n ("Offline"), Kopete::OnlineStatusManager::Offline, Kopete::OnlineStatusManager::HasStatusMessage ),
	JabberKOSInvisible(Kopete::OnlineStatus::Invisible,   40, this, JabberInvisible, QStringList(QStringLiteral("contact_invisible_overlay")),   i18n ("Invisible") ,i18n ("Invisible"), Kopete::OnlineStatusManager::Invisible),
	JabberKOSConnecting(Kopete::OnlineStatus::Connecting, 30, this, JabberConnecting, QStringList(QStringLiteral("jabber_connecting")),  i18n("Connecting")),
	propLastSeen(Kopete::Global::Properties::self()->lastSeen()),
	propFirstName(Kopete::Global::Properties::self()->firstName()),
	propLastName(Kopete::Global::Properties::self()->lastName()),
	propFullName(Kopete::Global::Properties::self()->fullName()),
	propEmailAddress(Kopete::Global::Properties::self()->emailAddress()),
	propPrivatePhone(Kopete::Global::Properties::self()->privatePhone()),
	propPrivateMobilePhone(Kopete::Global::Properties::self()->privateMobilePhone()),
	propWorkPhone(Kopete::Global::Properties::self()->workPhone()),
	propWorkMobilePhone(Kopete::Global::Properties::self()->workMobilePhone()),
	propNickName(Kopete::Global::Properties::self()->nickName()),
	propSubscriptionStatus(QStringLiteral("jabberSubscriptionStatus"), i18n ("Subscription"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propAuthorizationStatus(QStringLiteral("jabberAuthorizationStatus"), i18n ("Authorization Status"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propAvailableResources(QStringLiteral("jabberAvailableResources"), i18n ("Available Resources"), QStringLiteral("jabber_chatty"), Kopete::PropertyTmpl::RichTextProperty),
	propVCardCacheTimeStamp(QStringLiteral("jabberVCardCacheTimeStamp"), i18n ("vCard Cache Timestamp"), QString(), Kopete::PropertyTmpl::PersistentProperty | Kopete::PropertyTmpl::PrivateProperty),
	propPhoto(Kopete::Global::Properties::self()->photo()),
	propJid(QStringLiteral("jabberVCardJid"), i18n("Jabber ID"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propBirthday(QStringLiteral("jabberVCardBirthday"), i18n("Birthday"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propTimezone(QStringLiteral("jabberVCardTimezone"), i18n("Timezone"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propHomepage(QStringLiteral("jabberVCardHomepage"), i18n("Homepage"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propCompanyName(QStringLiteral("jabberVCardCompanyName"), i18n("Company name"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propCompanyDepartement(QStringLiteral("jabberVCardCompanyDepartement"), i18n("Company Departement"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propCompanyPosition(QStringLiteral("jabberVCardCompanyPosition"), i18n("Company Position"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propCompanyRole(QStringLiteral("jabberVCardCompanyRole"), i18n("Company Role"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkStreet(QStringLiteral("jabberVCardWorkStreet"), i18n("Work Street"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkExtAddr(QStringLiteral("jabberVCardWorkExtAddr"), i18n("Work Extra Address"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkPOBox(QStringLiteral("jabberVCardWorkPOBox"), i18n("Work PO Box"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkCity(QStringLiteral("jabberVCardWorkCity"), i18n("Work City"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkPostalCode(QStringLiteral("jabberVCardWorkPostalCode"), i18n("Work Postal Code"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkCountry(QStringLiteral("jabberVCardWorkCountry"), i18n("Work Country"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propWorkEmailAddress(QStringLiteral("jabberVCardWorkEmailAddress"), i18n("Work Email Address"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propHomeStreet(QStringLiteral("jabberVCardHomeStreet"), i18n("Home Street"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propHomeExtAddr(QStringLiteral("jabberVCardHomeExt"), i18n("Home Extra Address"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propHomePOBox(QStringLiteral("jabberVCardHomePOBox"), i18n("Home PO Box"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propHomeCity(QStringLiteral("jabberVCardHomeCity"), i18n("Home City"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propHomePostalCode(QStringLiteral("jabberVCardHomePostalCode"), i18n("Home Postal Code"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propHomeCountry(QStringLiteral("jabberVCardHomeCountry"), i18n("Home Country"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propPhoneFax(QStringLiteral("jabberVCardPhoneFax"), i18n("Fax"), QString(), Kopete::PropertyTmpl::PersistentProperty),
	propAbout(QStringLiteral("jabberVCardAbout"), i18n("About"), QString(), Kopete::PropertyTmpl::PersistentProperty)

{

	qDebug (JABBER_PROTOCOL_LOG) << "[JabberProtocol] Loading ...";

	/* This is meant to be a singleton, so we will check if we have
	 * been loaded before. */
	if (protocolInstance)
	{
		qDebug (JABBER_PROTOCOL_LOG) << "[JabberProtocol] Warning: Protocol already " << "loaded, not initializing again.";
		return;
	}

	protocolInstance = this;

	addAddressBookField (QStringLiteral("messaging/xmpp"), Kopete::Plugin::MakeIndexField);
	setCapabilities(Kopete::Protocol::FullRTF|Kopete::Protocol::CanSendOffline);

	// Init the Entity Capabilities manager.
	capsManager = new JabberCapabilitiesManager;
	capsManager->loadCachedInformation();
	
	registerAsProtocolHandler(QStringLiteral("xmpp"));
}

JabberProtocol::~JabberProtocol ()
{
	//disconnectAll();

	delete capsManager;
	capsManager = nullptr;

	/* make sure that the next attempt to load Jabber
	 * re-initializes the protocol class. */
	protocolInstance = nullptr;
}

AddContactPage *JabberProtocol::createAddContactWidget (QWidget * parent, Kopete::Account * i)
{
	qDebug (JABBER_PROTOCOL_LOG) << "Create Add Contact  Widget";
	return new JabberAddContactPage (i, parent);
}

KopeteEditAccountWidget *JabberProtocol::createEditAccountWidget (Kopete::Account * account, QWidget * parent)
{
	qDebug (JABBER_PROTOCOL_LOG) << "Edit Account Widget";
	JabberAccount *ja=dynamic_cast < JabberAccount * >(account);
	if(ja || !account)
		return new JabberEditAccountWidget (this,ja , parent);
	else
	{
		JabberTransport *transport = dynamic_cast < JabberTransport * >(account);
		if(!transport || !transport->account()->client() )
			return 0L;
		dlgRegister *registerDialog = new dlgRegister (transport->account(), transport->myself()->contactId());
		registerDialog->show (); 
		registerDialog->raise ();
		return 0l; //we make ourself our own dialog, not an editAccountWidget.
	}
}

Kopete::Account *JabberProtocol::createNewAccount (const QString & accountId)
{
	qDebug (JABBER_PROTOCOL_LOG) << "Create New Account. ID: " << accountId;
	if( Kopete::AccountManager::self()->findAccount( pluginId() , accountId ) )
		return 0L;  //the account may already exist if greated just above

	int slash=accountId.indexOf('/');
	if(slash>=0)
	{
		QString realAccountId=accountId.left(slash);
		JabberAccount *realAccount=dynamic_cast<JabberAccount*>(Kopete::AccountManager::self()->findAccount( pluginId() , realAccountId ));
		if(!realAccount) //if it doesn't exist yet, create it
		{
			realAccount = new JabberAccount( this, realAccountId );
			if(!Kopete::AccountManager::self()->registerAccount(  realAccount ) )
				return 0L;
		}
		if(!realAccount)
			return 0L;
		return new JabberTransport( realAccount , accountId );
	}
	else
	{
		return new JabberAccount (this, accountId);
	}
}

Kopete::OnlineStatus JabberProtocol::resourceToKOS ( const XMPP::Resource &resource )
{

	// update to offline by default
	Kopete::OnlineStatus status = JabberKOSOffline;

	if ( !resource.status().isAvailable () )
	{
		// resource is offline
		status = JabberKOSOffline;
	}
	else
	{
		if (resource.status ().show ().isEmpty ())
		{
			if (resource.status ().isInvisible ())
			{
				status = JabberKOSInvisible;
			}
			else
			{
				status = JabberKOSOnline;
			}
		}
		else
		if (resource.status ().show () == QLatin1String("chat"))
		{
			status = JabberKOSChatty;
		}
		else if (resource.status ().show () == QLatin1String("away"))
		{
			status = JabberKOSAway;
		}
		else if (resource.status ().show () == QLatin1String("xa"))
		{
			status = JabberKOSXA;
		}
		else if (resource.status ().show () == QLatin1String("dnd"))
		{
			status = JabberKOSDND;
		}
		else if (resource.status ().show () == QLatin1String("online"))
		{ // the ApaSMSAgent sms gateway report status as "online" even if it's not in the RFC 3921 � 2.2.2.1 
			// See Bug 129059
			status = JabberKOSOnline;
		}
		else if (resource.status ().show () == QLatin1String("connecting"))
		{ // this is for kopete internals
			status = JabberKOSConnecting;
		}
		else
		{
			status = JabberKOSOnline; // Assume that contact is online bug 210558
			qDebug (JABBER_PROTOCOL_LOG) << "Unknown status <show>" << resource.status ().show () << "</show> for contact. One of your contact is probably using a broken client, ask him to report a bug";
		}
	}

	return status;

}

JabberCapabilitiesManager *JabberProtocol::capabilitiesManager()
{
	return capsManager;
}

JabberProtocol *JabberProtocol::protocol ()
{
	// return current instance
	return protocolInstance;
}

Kopete::Contact *JabberProtocol::deserializeContact (Kopete::MetaContact * metaContact,
										 const QMap < QString, QString > &serializedData, const QMap < QString, QString > & /* addressBookData */ )
{
//  qDebug (JABBER_PROTOCOL_LOG) << "Deserializing data for metacontact " << metaContact->displayName () << "\n";

	QString contactId = serializedData[QStringLiteral("contactId")];
	QString accountId = serializedData[QStringLiteral("accountId")];
	Kopete::Contact::NameType nameType = Kopete::Contact::nameTypeFromString(serializedData[ QStringLiteral("preferredNameType") ]);
	QString jid = serializedData[QStringLiteral("JID")];

	QList<Kopete::Account*> accounts = Kopete::AccountManager::self ()->accounts (this);
	Kopete::Account *account = 0;
	QList<Kopete::Account*>::Iterator accountIt, accountItEnd = accounts.end();
	for(accountIt = accounts.begin(); accountIt != accountItEnd; ++accountIt)
	{
		if((*accountIt)->accountId() == accountId)
			account = *accountIt;
	}

	if (!account)
	{
		qCDebug(JABBER_PROTOCOL_LOG) << "WARNING: Account for contact does not exist, skipping.";
		return 0;
	}
	
	JabberTransport *transport = dynamic_cast<JabberTransport*>(account);
	if( transport )
		transport->account()->addContact ( jid.isEmpty() ? contactId : jid ,  metaContact);
	else
		account->addContact (contactId,  metaContact);

	Kopete::Contact *c = account->contacts().value(contactId);
	if (!c)
		return NULL;

	c->setPreferredNameType(nameType);
	return c;
}

XMPP::Status JabberProtocol::kosToStatus( const Kopete::OnlineStatus & status , const QString & message )
{
	XMPP::Status xmppStatus ( QLatin1String(""), message );

	if( status.status() == Kopete::OnlineStatus::Offline )
	{
		xmppStatus.setIsAvailable( false );
	}

	switch ( status.internalStatus () )
	{
		case JabberProtocol::JabberFreeForChat:
			xmppStatus.setShow ( QStringLiteral("chat") );
			break;

		case JabberProtocol::JabberOnline:
			xmppStatus.setShow ( QLatin1String("") );
			break;

		case JabberProtocol::JabberAway:
			xmppStatus.setShow ( QStringLiteral("away") );
			break;

		case JabberProtocol::JabberXA:
			xmppStatus.setShow ( QStringLiteral("xa") );
			break;

		case JabberProtocol::JabberDND:
			xmppStatus.setShow ( QStringLiteral("dnd") );
			break;

		case JabberProtocol::JabberInvisible:
			xmppStatus.setIsInvisible ( true );
			break;
	}
	return xmppStatus;
}

#include <accountselector.h>
#include <kopeteuiglobal.h>
#include <QVBoxLayout>
#include "jabbercontactpool.h"
#include <kopeteview.h>
#include <kmessagebox.h>
#include <qinputdialog.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

void JabberProtocol::handleURL(const QString&, const QUrl &url) const
{
	if(url.scheme() != QLatin1String("xmpp") && !url.scheme().isEmpty() )
		return;

	QUrlQuery query(url);
	query.setQueryDelimiters( '=' , ';' );
	QString accountid=url.authority();
	QString jid_str=url.path();
	if(jid_str.startsWith('/'))
		jid_str=jid_str.mid(1);
	XMPP::Jid jid = jid_str;
	QString action=query.queryItems().isEmpty() ? QString() : query.queryItems().first().first;
	 
	qCDebug(JABBER_PROTOCOL_LOG) << query.queryItemValue(QStringLiteral("body"));

	if(jid.isEmpty())
	{
		return;
	}
	
	JabberAccount *account=0L;
	if(!accountid.isEmpty())
	{
		account=static_cast<JabberAccount*>(Kopete::AccountManager::self()->findAccount(QStringLiteral("JabberProtocol") , accountid));
	}
	if(!account)
	{
		QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts(const_cast<JabberProtocol*>(this));
		if (accounts.count() == 1)
			account = static_cast<JabberAccount*>(accounts.first());
		else
		{
			QPointer <QDialog> chooser = new QDialog(Kopete::UI::Global::mainWidget());
			chooser->setWindowTitle( i18n("Choose Account") );
			QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
			QWidget *mainWidget = new QWidget(chooser);
			QVBoxLayout *mainLayout = new QVBoxLayout;
			chooser->setLayout(mainLayout);
			mainLayout->addWidget(mainWidget);
			QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
			okButton->setDefault(true);
			okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
			chooser->connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
			chooser->connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
			buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
			QLabel * label = new QLabel(mainWidget);
			mainLayout->addWidget(label);
			label->setText(i18n("Choose an account to handle the URL %1" , url.toDisplayString()));
//			label->setSizePolicy(QSizePolicy::Minimum , QSizePolicy::MinimumExpanding);
			label->setWordWrap(true);
			QPointer <AccountSelector> accSelector = new AccountSelector(const_cast<JabberProtocol*>(this), mainWidget);
			mainLayout->addWidget(accSelector);
			mainLayout->addWidget(buttonBox);
	//		accSelector->setSizePolicy(QSizePolicy::MinimumExpanding , QSizePolicy::MinimumExpanding);
			int ret = chooser->exec();
			if (ret == QDialog::Rejected || !accSelector)
			{
				delete chooser;
				delete accSelector;
				return;
			}
			account=qobject_cast<JabberAccount*>(accSelector->selectedItem());
			delete chooser;
			delete accSelector;
			if (!account)
				return;
		}
	}
	
	if(jid.isEmpty())
	{
		return;
	}
	
	JabberBaseContact *contact=account->contactPool()->findRelevantRecipient( jid );
		
	if(action.isEmpty() || action==QLatin1String("message"))
	{
		if(!contact)
		{
			Kopete::MetaContact *metaContact = new Kopete::MetaContact ();
			metaContact->setTemporary (true);
			contact = account->contactPool()->addContact ( XMPP::RosterItem ( jid ), metaContact, false );
			Kopete::ContactList::self()->addMetaContact(metaContact);
		}
		contact->execute();
		
		if(url.hasQueryItem(QStringLiteral("body")) || url.hasQueryItem(QStringLiteral("subject")))
		{ //TODO "thread"
			Kopete::ChatSession *kcs=contact->manager(Kopete::Contact::CanCreate);
			if(!kcs)
				return;

			Kopete::Message msg(account->myself(), kcs->members());
			msg.setPlainBody( url.queryItemValue(QStringLiteral("body")) );
			msg.setSubject( url.queryItemValue(QStringLiteral("subject")) );
			msg.setDirection( Kopete::Message::Outbound );

			KopeteView *kv=kcs->view(true);
			if(kv)
				kv->setCurrentMessage(msg);
		}
	}
	else if(action == QLatin1String("roster") || action == QLatin1String("subscribe"))
	{
		if (KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(),
			i18n("Do you want to add '%1' to your contact list?", jid.full()),
			QString(), KGuiItem( i18n("Add") ), KGuiItem( i18n("Do Not Add") ))
				  != KMessageBox::Yes)
		{
			return;
		}
		Kopete::Group *group=0L;
		if( url.hasQueryItem(QStringLiteral("group")))
			group=Kopete::ContactList::self()->findGroup( url.queryItemValue(QStringLiteral("group")) );
		account->addContact( jid.full() ,  url.queryItemValue(QStringLiteral("name")) , group );
	}
	else if(action == QLatin1String("remove") || action== QLatin1String("unsubscribe"))
	{
		if(!contact)
			return;
	
		if (KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(),
			i18n("Do you want to remove '%1' from your contact list?", jid.full()),
			QString(), KGuiItem( i18n("Remove") ), KGuiItem( i18n("Do Not Remove") ))
						!= KMessageBox::Yes)
		{
			return;
		}
		
		Kopete::DeleteContactTask *deleteTask = new Kopete::DeleteContactTask(contact);
		deleteTask->start();
	}//TODO: probe
	else if(action == QLatin1String("join") || action==QLatin1String("invite"))
	{
		if(!account->isConnected() || !account->client())
		{
			account->errorConnectFirst();
			return;
		}
		
		if(!contact)
		{
			QString nick=jid.resource();
			if(nick.isEmpty())
			{
				bool ok=true;
                nick = QInputDialog::getText(nullptr, i18n("Please enter your nickname for the room %1", jid.bare()),
                        i18n("Provide your nickname"), QLineEdit::Normal,
						QString(),
						&ok);
				if (!ok)
					return;
			}
			if(action==QLatin1String("join") && url.hasQueryItem(QStringLiteral("password")))
				account->client()->joinGroupChat( jid.domain() , jid.node() , nick, url.queryItemValue(QStringLiteral("password")) );
			else
				account->client()->joinGroupChat( jid.domain() , jid.node() , nick );
		}
		
		if(action==QLatin1String("invite") && url.hasQueryItem(QStringLiteral("jid")) )
		{
			//NOTE: this is the obsolete, NOT RECOMMENDED protocol.
			//      iris doesn't implement groupchat yet
			//NOTE: This code is duplicated in JabberGroupChatManager::inviteContact
			XMPP::Message jabberMessage;
			//XMPP::Jid jid = static_cast<const JabberBaseContact*>(account->myself())->rosterItem().jid() ;
			//jabberMessage.setFrom ( jid );
			jabberMessage.setTo ( url.queryItemValue(QStringLiteral("jid")) );
			jabberMessage.setInvite( jid.bare() );
			jabberMessage.setBody( i18n("You have been invited to %1", jid.bare() ) );

			// send the message
			account->client()->sendMessage ( jabberMessage );
		}
	}
	else if(action==QLatin1String("sendfile"))
	{
		if(!contact)
		{
			Kopete::MetaContact *metaContact = new Kopete::MetaContact ();
			metaContact->setTemporary (true);
			contact = account->contactPool()->addContact ( XMPP::RosterItem ( jid ), metaContact, false );
			Kopete::ContactList::self()->addMetaContact(metaContact);
		}
		contact->sendFile();
	}//TODO: recvfile
	else
	{
		qCWarning(JABBER_PROTOCOL_LOG) << "unable to handle URL "<< url.toDisplayString();
	}

}

#include "jabberprotocol.moc"
