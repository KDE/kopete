/*
    Messenger Add contact UI
    Copyright (c) 2007 by pyzhang <pyzhang@gmail.com>

	Kopete    (c) 2002-2007 by The Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qlayout.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "kopeteaccount.h"
#include "kopeteuiglobal.h"

#include "ui_messengeradd.h"

#include "messengeraddcontactpage.h"
#include "messengerprotocol.h"

MessengerAddContactPage::MessengerAddContactPage(MessengerAccount * owner, QWidget *parent)
				  : AddContactPage(parent)
{
	kdebug(14166) << k_funcinfo << "called" << endl;

	m_account = owner;
	addUI = new Ui::MessengerAddUI();
	addUI->setupUi( this );
}

MessengerAddContactPage::~MessengerAddContactPage()
{
	delete addUI;
}

bool MessengerAddContactPage::apply( Kopete::Account* , Kopete::MetaContact*m )
{
	kdebug(14166) << k_funcinfo << "called" << endl;

	if ( validateData())
	{
		QString userid = addUI->addID->text();
		return i->addContact( userid , m, Kopete::Account::ChangeKABC );
	}
	return false;
}

bool MessengerAddContactPage::validateData()
{
	if(!mAccount->isConnected())
	{
		//Account currently offline
		KMessageBox::sorry( this, i18n("You must be online to add a contact."), i18n("messenger Plugin") );
		return false;
	}

	QString userid = addUI->addID->text();

	if(MessengerProtocol::validContactId(userid))
	{
		return true;
	}

	KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
			i18n( "<qt>You must enter a valid email address.</qt>" ), i18n( "MSN Plugin" )  );
	return false;
}

#include "messengeraddcontactpage.moc"

// vim: set noet ts=4 sts=4 sw=4:

