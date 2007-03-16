/*
  oscarprotocol.cpp  -  Oscar Protocol Plugin

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

#include <qstringlist.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "aimprotocol.h"
#include "aimaccount.h"
#include "aimcontact.h"
#include "aimaddcontactpage.h"
#include "aimeditaccountwidget.h"

#include "accountselector.h"
#include "kopeteaccountmanager.h"
#include "kopeteonlinestatusmanager.h"
#include "kopeteglobal.h"
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"

#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kimageio.h>

typedef KGenericFactory<AIMProtocol> AIMProtocolFactory;

K_EXPORT_COMPONENT_FACTORY( kopete_aim, AIMProtocolFactory( "kopete_aim" ) )

AIMProtocol* AIMProtocol::protocolStatic_ = 0L;


AIMProtocolHandler::AIMProtocolHandler() : Kopete::MimeTypeHandler(false)
{
	registerAsProtocolHandler(QString::fromLatin1("aim"));
}

void AIMProtocolHandler::handleURL(const KURL &url) const
{
/**
 * Send a Message  =================================================
 * aim:goim
 * aim:goim?screenname=screen+name
 * aim:goim?screenname=screen+name&message=message
 * Add Buddy  ======================================================
 * aim:addbuddy
 * aim:addbuddy?screenname=screen+name
 * Buddy Icon  =====================================================
 * aim:buddyicon
 * aim:buddyicon?src=image_source
 * aim:buddyicon?screename=screen+name
 * aim:buddyicon?src=image_source&screename=screen+name
 * Get and Send Files  =============================================
 * aim:getfile?screename=(sn)
 * aim:sendfile?screenname=(sn)
 * Register User  ==================================================
 * aim:RegisterUser?ScreenName=sn&Password=pw&SignonNow=False
 * Away Message  ===================================================
 * aim:goaway?message=brb+or+something
 * Chat Rooms  =====================================================
 * aim:GoChat?RoomName=room+name&Exchange=number
 **/

	AIMProtocol *proto = AIMProtocol::protocol();
	kdDebug(14152) << k_funcinfo << "URL url   : '" << url.url() << "'" << endl;
	kdDebug(14152) << k_funcinfo << "URL path  : '" << url.path() << "'" << endl;
	QString command = url.path();
	QString realCommand, firstParam, secondParam;
	bool needContactAddition = false;
	if ( command.find( "goim", 0, false ) != -1 )
	{
		realCommand = "goim";
		kdDebug(14152) << k_funcinfo << "Handling send IM request" << endl;
		command.remove(0,4);
		if ( command.find( "?screenname=", 0, false ) == -1 )
		{
		kdWarning(14152) << k_funcinfo << "Unhandled AIM URI:" << url.url() << endl;
			return;
		}
		command.remove( 0, 12 );
		int andSign = command.find( "&" );
		if ( andSign > 0 )
			command = command.left( andSign );

		firstParam = command;
		firstParam.replace( "+", " " );
		needContactAddition = true;
	}
	else
		if ( command.find( "addbuddy", 0, false ) != -1 )
		{
			realCommand = "addbuddy";
			kdDebug(14152) << k_funcinfo << "Handling AIM add buddy request" << endl;
			command.remove( 0, 8 );
			if ( command.find( "?screenname=", 0, false ) == -1 )
			{
			kdWarning(14152) << k_funcinfo << "Unhandled AIM URI:" << url.url() << endl;
				return;
			}
			
			command.remove(0, 12);
			int andSign = command.find("&");
			if ( andSign > 0 )
				command = command.left(andSign);
			command.replace("+", " ");
			
			firstParam = command;
			needContactAddition = true;
		}
	else
	if ( command.find( "gochat", 0, false ) != -1 )
	{
		realCommand = "gochat";
		kdDebug(14152) << k_funcinfo << "Handling AIM chat room request" << endl;
		command.remove( 0, 6 );
		
		if ( command.find( "?RoomName=", 0, false ) == -1 )
		{
		kdWarning(14152) << "Unhandled AIM URI: " << url.url() << endl;
			return;
		}
		
		command.remove( 0, 10 );
		
		int andSign = command.find("&");
		if (andSign > 0) // strip off anything else for now
		{
			firstParam = command.left(andSign);
		}
		command.remove( 0, andSign );
		kdDebug(14152) << "command is now: " << command << endl;
		command.remove( 0, 10 ); //remove "&Exchange="
		secondParam = command;
		kdDebug(14152) << k_funcinfo << firstParam << " " << secondParam << endl;
		firstParam.replace("+", " ");
	}
	
	Kopete::Account *account = 0;
	QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts(proto);
	
	if (accounts.count() == 1)
	{
		QDictIterator<Kopete::Account> it(accounts);
		account = it.current();
		
	}
	else
	{
		KDialogBase *chooser = new KDialogBase(0, "chooser", true,
		                                       i18n("Choose Account"), KDialogBase::Ok|KDialogBase::Cancel,
		                                       KDialogBase::Ok, false);
		AccountSelector *accSelector = new AccountSelector(proto, chooser, "accSelector");
		chooser->setMainWidget(accSelector);
		
		int ret = chooser->exec();
		account = accSelector->selectedItem();
		
		delete chooser;
		if (ret == QDialog::Rejected || account == 0)
		{
			kdDebug(14152) << k_funcinfo << "Cancelled" << endl;
			return;
		}
	}
	
	Kopete::MetaContact* mc = 0;
	if ( needContactAddition || realCommand == "addbuddy" )
	{
		if ( !account->isConnected() )
		{
			kdDebug(14152) << k_funcinfo << "Can't add contact, we are offline!" << endl;
			KMessageBox::sorry( Kopete::UI::Global::mainWidget(), i18n("You need to be connected to be able to add contacts."),
			                    i18n("AIM") );
			return;
		}

		if (KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(),
		                               i18n("Do you want to add '%1' to your contact list?").arg(command),
		                               QString::null, i18n("Add"), i18n("Do Not Add"))
		    != KMessageBox::Yes)
		{
			kdDebug(14152) << k_funcinfo << "Cancelled" << endl;
			return;
		}
		
		kdDebug(14152) << k_funcinfo <<
			"Adding Contact; screenname = " << firstParam << endl;
		mc = account->addContact(firstParam, command, 0L, Kopete::Account::Temporary);
	}

	if ( realCommand == "gochat" )
	{
		AIMAccount* aimAccount = dynamic_cast<AIMAccount*>( account );
		if ( aimAccount && aimAccount->isConnected() )
		{
			aimAccount->engine()->joinChatRoom( firstParam, secondParam.toInt() );
		}
		else
			KMessageBox::sorry( Kopete::UI::Global::mainWidget(),
			                    i18n( "Unable to connect to the chat room %1 because the account"
			                          " for %2 is not connected." ).arg( firstParam ).arg( aimAccount->accountId() ),
			                    QString::null );
		
	}

	if ( mc && realCommand == "goim" )
	{
		mc->execute();
	}
	
}




AIMProtocol::AIMProtocol(QObject *parent, const char *name, const QStringList &)
  : Kopete::Protocol( AIMProtocolFactory::instance(), parent, name ),
	statusOnline( Kopete::OnlineStatus::Online, 2, this, 0, QString::null, i18n("Online"), i18n("Online"), Kopete::OnlineStatusManager::Online ),
	statusOffline( Kopete::OnlineStatus::Offline, 2, this, 10, QString::null, i18n("Offline"), i18n("Offline"), Kopete::OnlineStatusManager::Offline ),
	statusAway( Kopete::OnlineStatus::Away, 2, this, 20, "contact_away_overlay", i18n("Away"), i18n("Away"), Kopete::OnlineStatusManager::Away,
							Kopete::OnlineStatusManager::HasAwayMessage ),
	statusWirelessOnline( Kopete::OnlineStatus::Online, 1, this, 30, "contact_phone_overlay", i18n("Mobile"), i18n("Mobile"),
	Kopete::OnlineStatusManager::Online, Kopete::OnlineStatusManager::HideFromMenu ),
	statusWirelessAway( Kopete::OnlineStatus::Away, 1, this, 31, QStringList::split( " ", "contact_phone_overlay contact_away_overlay"),
	i18n("Mobile Away"), i18n("Mobile Away"), Kopete::OnlineStatusManager::Away, Kopete::OnlineStatusManager::HideFromMenu ),
	statusConnecting(Kopete::OnlineStatus::Connecting, 99, this, 99, "aim_connecting", i18n("Connecting...")),
	awayMessage(Kopete::Global::Properties::self()->awayMessage()),
	clientFeatures("clientFeatures", i18n("Client Features"), 0, false),
	clientProfile( "clientProfile", i18n( "User Profile"), 0, false, true),
	iconHash("iconHash", i18n("Buddy Icon MD5 Hash"), QString::null, true, false, true)
{
	if (protocolStatic_)
		kdDebug(14152) << k_funcinfo << "AIM plugin already initialized" << endl;
	else
		protocolStatic_ = this;

	setCapabilities(0x1FFF); // setting capabilities - FIXME to use proper enum
	kdDebug(14152) << k_funcinfo << "capabilities set to 0x1FFF" << endl;
	addAddressBookField("messaging/aim", Kopete::Plugin::MakeIndexField);
	KImageIO::registerFormats();
}

AIMProtocol::~AIMProtocol()
{
	protocolStatic_ =0L;
}

AIMProtocol *AIMProtocol::protocol(void)
{
	return protocolStatic_;
}

Kopete::Contact *AIMProtocol::deserializeContact(Kopete::MetaContact *metaContact,
    const QMap<QString, QString> &serializedData,
    const QMap<QString, QString> &/*addressBookData*/)
{

	QString contactId = serializedData["contactId"];
	QString accountId = serializedData["accountId"];
	QString displayName = serializedData["displayName"];

	// Get the account it belongs to
	QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts( this );
	Kopete::Account *account = accounts[accountId];

	if ( !account ) //no account
		return 0;

	uint ssiGid = 0, ssiBid = 0, ssiType = 0xFFFF;
	QString ssiName;
	bool ssiWaitingAuth = false;
	if ( serializedData.contains( "ssi_type" ) )
	{
		ssiName = serializedData["ssi_name"];
		QString authStatus = serializedData["ssi_waitingAuth"];
		if ( authStatus == "true" )
		ssiWaitingAuth = true;
		ssiGid = serializedData["ssi_gid"].toUInt();
		ssiBid = serializedData["ssi_bid"].toUInt();
		ssiType = serializedData["ssi_type"].toUInt();
	}

	Oscar::SSI item( ssiName, ssiGid, ssiBid, ssiType, QValueList<TLV>(), 0 );
	item.setWaitingAuth( ssiWaitingAuth );

	AIMContact *c = new AIMContact( account, contactId, metaContact, QString::null, item );
	return c;
}

AddContactPage *AIMProtocol::createAddContactWidget(QWidget *parent, Kopete::Account *account)
{
	return ( new AIMAddContactPage( account->isConnected(), parent ) );
}

KopeteEditAccountWidget *AIMProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent)
{
	return ( new AIMEditAccountWidget( this, account, parent ) );
}

Kopete::Account *AIMProtocol::createNewAccount(const QString &accountId)
{
	return ( new AIMAccount( this, accountId ) );
}

#include "aimprotocol.moc"
// vim: set noet ts=4 sts=4 sw=4:
