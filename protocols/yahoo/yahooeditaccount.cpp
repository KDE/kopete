/*
    yahooeditaccount.cpp - UI Page to edit a Yahoo account

    Copyright (c) 2003 by Matt Rogers <mattrogers@sbcglobal.net>
    Copyright (c) 2002 by Gav Wood <gav@kde.org>

    Copyright (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

// QT Includes
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qlayout.h>

// KDE Includes
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

// Kopete Includes
#include <addcontactpage.h>

// Local Includes
#include "yahooaccount.h"
#include "yahoocontact.h"
#include "yahooeditaccount.h"

// Yahoo Add Contact page
YahooEditAccount::YahooEditAccount(YahooProtocol *protocol, KopeteAccount *theAccount, QWidget *parent, const char* /*name*/): YahooEditAccountBase(parent), KopeteEditAccountWidget(theAccount)
{
	kdDebug(14180) << k_funcinfo << endl;

	theProtocol = protocol;

	mPasswordWidget = new Kopete::UI::PasswordWidget( mAccountInfo );
	mAccountInfoLayout->add( mPasswordWidget );

	if(YahooAccount *acct = dynamic_cast<YahooAccount*>(account()))
	{	mScreenName->setText(acct->accountId());
		mScreenName->setReadOnly(true); //the accountId is Constant FIXME: remove soon!
		mScreenName->setDisabled(true);
		mAutoConnect->setChecked(acct->autoLogin());
		mPasswordWidget->load( &acct->password() );
	}

	show();
}

bool YahooEditAccount::validateData()
{
	kdDebug(14180) << k_funcinfo << endl;

	if(mScreenName->text().isEmpty())
	{	KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
			i18n("<qt>You must enter a valid screen name.</qt>"), i18n("Yahoo"));
		return false;
	}
	if(!mPasswordWidget->validate())
	{	KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
			i18n("<qt>You must enter a valid password.</qt>"), i18n("Yahoo"));
		return false;
	}
	return true;
}

KopeteAccount *YahooEditAccount::apply()
{
	kdDebug(14180) << k_funcinfo << endl;

	if ( !account() )
		setAccount( new YahooAccount( theProtocol, mScreenName->text() ) );

	YahooAccount *yahooAccount = static_cast<YahooAccount *>( account() );

	yahooAccount->setAutoLogin( mAutoConnect->isChecked() );

	mPasswordWidget->save( &yahooAccount->password() );

	return yahooAccount;
}

#include "yahooeditaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

