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

#include "aimprotocol.h"

#include <kgenericfactory.h>

#include "aimaccount.h"
#include "aimstatusmanager.h"
#include "aimaddcontactpage.h"
#include "aimeditaccountwidget.h"

#include "accountselector.h"
#include "kopeteaccountmanager.h"
#include "kopeteglobal.h"
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"

#include <kdialog.h>
#include <kmessagebox.h>

K_PLUGIN_FACTORY( AIMProtocolFactory, registerPlugin<AIMProtocol>(); )
K_EXPORT_PLUGIN( AIMProtocolFactory( "kopete_aim" ) )

AIMProtocol* AIMProtocol::protocolStatic_ = 0L;


AIMProtocolHandler::AIMProtocolHandler() : Kopete::MimeTypeHandler(false)
{
	registerAsProtocolHandler(QString::fromLatin1("aim"));
}

void AIMProtocolHandler::handleURL(const QString&, const KUrl &url) const
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
	kDebug(14152) << "URL url   : '" << url.url() << "'";
	kDebug(14152) << "URL path  : '" << url.path() << "'";
	QString command = url.path();
	QString realCommand, firstParam, secondParam;
	bool needContactAddition = false;
	if ( command.indexOf( "goim", 0, Qt::CaseInsensitive ) != -1 )
	{
		realCommand = "goim";
		kDebug(14152) << "Handling send IM request";
		command.remove(0,4);
		if ( command.indexOf( "?screenname=", 0, Qt::CaseInsensitive ) == -1 )
		{
		kWarning(14152) << "Unhandled AIM URI:" << url.url();
			return;
		}
		command.remove( 0, 12 );
		int andSign = command.indexOf( '&' );
		if ( andSign > 0 )
			command = command.left( andSign );

		firstParam = command;
		firstParam.replace( '+', ' ' );
		needContactAddition = true;
	}
	else
		if ( command.indexOf( "addbuddy", 0, Qt::CaseInsensitive ) != -1 )
		{
			realCommand = "addbuddy";
			kDebug(14152) << "Handling AIM add buddy request";
			command.remove( 0, 8 );
			if ( command.indexOf( "?screenname=", 0, Qt::CaseInsensitive ) == -1 )
			{
			kWarning(14152) << "Unhandled AIM URI:" << url.url();
				return;
			}

			command.remove(0, 12);
			int andSign = command.indexOf('&');
			if ( andSign > 0 )
				command = command.left(andSign);
			command.replace('+', ' ');

			firstParam = command;
			needContactAddition = true;
		}
	else
	if ( command.indexOf( "gochat", 0, Qt::CaseInsensitive ) != -1 )
	{
		realCommand = "gochat";
		kDebug(14152) << "Handling AIM chat room request";
		command.remove( 0, 6 );

		if ( command.indexOf( "?RoomName=", 0, Qt::CaseInsensitive ) == -1 )
		{
		kWarning(14152) << "Unhandled AIM URI: " << url.url();
			return;
		}

		command.remove( 0, 10 );

		int andSign = command.indexOf('&');
		if (andSign > 0) // strip off anything else for now
		{
			firstParam = command.left(andSign);
		}
		command.remove( 0, andSign );
		kDebug(14152) << "command is now: " << command;
		command.remove( 0, 10 ); //remove "&Exchange="
		secondParam = command;
		kDebug(14152) << firstParam << " " << secondParam;
		firstParam.replace('+', ' ');
	}

	Kopete::Account *account = 0;
	QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts();

	if (accounts.count() == 1)
	{
		account = accounts.first();
	}
	else
	{
		KDialog *chooser = new KDialog;
		chooser->setCaption( i18n("Choose Account") );
		chooser->setButtons( KDialog::Ok | KDialog::Cancel );
		chooser->setDefaultButton(KDialog::Ok);
		AccountSelector *accSelector = new AccountSelector(proto, chooser);
		accSelector->setObjectName( QLatin1String("accSelector") );
		chooser->setMainWidget(accSelector);

		int ret = chooser->exec();
		account = accSelector->selectedItem();

		delete chooser;
		if (ret == QDialog::Rejected || account == 0)
		{
			kDebug(14152) << "Cancelled";
			return;
		}
	}

	Kopete::MetaContact* mc = 0;
	if ( needContactAddition || realCommand == "addbuddy" )
	{
		if ( !account->isConnected() )
		{
			kDebug(14152) << "Can't add contact, we are offline!";
			KMessageBox::sorry( Kopete::UI::Global::mainWidget(), i18n("You need to be connected to be able to add contacts."),
			                    i18n("AIM") );
			return;
		}

		if (KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(),
		                               i18n("Do you want to add '%1' to your contact list?", command),
		                               QString(), KGuiItem( i18n("Add") ), KGuiItem( i18n("Do Not Add") ))
		    != KMessageBox::Yes)
		{
			kDebug(14152) << "Cancelled";
			return;
		}

		kDebug(14152) <<
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
		else if ( aimAccount )
		{
			KMessageBox::sorry( Kopete::UI::Global::mainWidget(),
			                    i18n( "Unable to connect to the chat room %1 because the account"
			                          " for %2 is not connected.", firstParam, aimAccount->accountId() ),
			                    QString() );
		}
	}

	if ( mc && realCommand == "goim" )
	{
		mc->execute();
	}

}




AIMProtocol::AIMProtocol(QObject *parent, const QVariantList &)
: OscarProtocol( AIMProtocolFactory::componentData(), parent, true ),
	clientProfile( "clientProfile", i18n( "User Profile"), 0, Kopete::PropertyTmpl::RichTextProperty)
{
	if (protocolStatic_)
		kDebug(14152) << "AIM plugin already initialized";
	else
		protocolStatic_ = this;

	// must be done after protocolStatic_ is set...
	statusManager_ = new AIMStatusManager;

	setCapabilities( Kopete::Protocol::FullRTF ); // setting capabilities
	kDebug(14152) << "capabilities set to FullRTF";
	addAddressBookField("messaging/aim", Kopete::Plugin::MakeIndexField);

}

AIMProtocol::~AIMProtocol()
{
	delete statusManager_;
	protocolStatic_ =0L;
}

AIMProtocol *AIMProtocol::protocol(void)
{
	return protocolStatic_;
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

OscarStatusManager *AIMProtocol::statusManager() const
{
	return statusManager_;
}

#include "aimprotocol.moc"
// vim: set noet ts=4 sts=4 sw=4:
