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


#include <kgenericfactory.h>
#include <kdebug.h>

#include "aimprotocol.h"
#include "aimaccount.h"
#include "aimcontact.h"
#include "aimaddcontactpage.h"
#include "aimeditaccountwidget.h"

#include "accountselector.h"
#include "kopeteaccountmanager.h"
#include "kopeteglobal.h"
#include "kopeteuiglobal.h"

#include <kdialogbase.h>
#include <kmessagebox.h>

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
 **/

	AIMProtocol *proto = AIMProtocol::protocol();
	kdDebug(14152) << k_funcinfo << "URL url   : '" << url.url() << "'" << endl;
	QString command = url.path();

	if (command.startsWith("goim") || command.startsWith("addbuddy"))
	{
		if (command.startsWith("goim"))
			command.remove(0,4);
		else
			command.remove(0,8);

		if (!command.startsWith("?screenname="))
		{
			kdWarning(14152) << "Unhandled aim URI : " << url.url() << endl;
			return;
		}

		command.remove(0, 12);

		int andSign = command.find("&");
		if (andSign > 0) // strip off anything else for now
			command = command.left(andSign);
		command.replace("+", " ");

		QString screenname = command;

		Kopete::Account *account = 0;
		QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts(proto);
		// do not show chooser if we only have one account to "choose" from
		if (accounts.count() == 1)
		{
			QDictIterator<Kopete::Account> it(accounts);
			account = it.current();

			if (KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(),
				i18n("Do you want to add '%1' to your contact list?").arg(command))
				!= KMessageBox::Yes)
			{
				kdDebug(14152) << k_funcinfo << "Cancelled" << endl;
				return;
			}
		}
		else
		{
			KDialogBase *chooser = new KDialogBase(0, "chooser", true,
				i18n("Choose Account"), KDialogBase::Ok|KDialogBase::Cancel,
				KDialogBase::Ok, false);
			AccountSelector *accSelector = new AccountSelector(proto, chooser,
				"accSelector");
			chooser->setMainWidget(accSelector);

			int ret = chooser->exec();
			Kopete::Account *account = accSelector->selectedItem();

			delete chooser;
			if (ret == QDialog::Rejected || account == 0)
			{
				kdDebug(14152) << k_funcinfo << "Cancelled" << endl;
				return;
			}
		}


		kdDebug(14152) << k_funcinfo <<
			"Adding Contact; screenname = " << screenname << endl;
		if ( account->addContact(screenname, command, 0L, Kopete::Account::Temporary) )
		{
			//Kopete::Contact *contact = account->contacts()[screenname];
		}


	}
	else
	{
		kdWarning(14152) << "Unhandled aim URI : " << url.url() << endl;
	}
}




AIMProtocol::AIMProtocol(QObject *parent, const char *name, const QStringList &)
  : Kopete::Protocol( AIMProtocolFactory::instance(), parent, name ),
	statusOnline(Kopete::OnlineStatus::Online, 1, this, 0, QString::null, i18n("Online")),
	statusOffline(Kopete::OnlineStatus::Offline, 1, this, 10, QString::null, i18n("Offline")),
	statusAway(Kopete::OnlineStatus::Away, 1, this, 20, "aim_away", i18n("Away")),
	statusConnecting(Kopete::OnlineStatus::Connecting, 99, this, 99, "aim_connecting", i18n("Connecting...")),
	awayMessage(Kopete::Global::Properties::self()->awayMessage()),
	clientFeatures("clientFeatures", i18n("Client Features"), 0, false),
	clientProfile( "clientProfile", i18n( "User Profile"), 0, false)
{
	if (protocolStatic_)
		kdDebug(14152) << k_funcinfo << "AIM plugin already initialized" << endl;
	else
		protocolStatic_ = this;

	addAddressBookField("messaging/aim", Kopete::Plugin::MakeIndexField);
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

	AIMContact *c = new AIMContact( account, contactId, metaContact, QString::null );
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
