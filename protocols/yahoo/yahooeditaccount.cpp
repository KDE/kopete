/***************************************************************************
                          yahooaddcontact.cpp  -  description
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@kde.org

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// QT Includes
#include <qwidget.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qcheckbox.h>

// KDE Includes
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klistbox.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kmessagebox.h>

// Kopete Includes
#include <addcontactpage.h>
#include <kopeteaccount.h>

// Local Includes
#include "yahooadd.h"
#include "yahooprotocol.h"
#include "yahooaccount.h"
#include "yahoocontact.h"
#include "yahooeditaccount.h"

// Yahoo Add Contact page
YahooEditAccount::YahooEditAccount(YahooProtocol *protocol, KopeteAccount *theAccount, QWidget *parent, const char *name): YahooEditAccountBase(parent), EditAccountWidget(theAccount)
{
	kdDebug(14180) << "YahooEditAccount::YahooEditAccount(<protocol>, <theAccount>, <parent>, " << name << ")";

	theProtocol = protocol;
	if(m_account)
	{	mScreenName->setText(m_account->accountId());
		if(m_account->rememberPassword())
			mPassword->setText(m_account->getPassword());
		mAutoConnect->setChecked(m_account->autoLogin());
		mRememberPassword->setChecked(true);
	}
	show();
}

bool YahooEditAccount::validateData()
{
	kdDebug(14180) << "YahooEditAccount::validateData()";
	
	if(mScreenName->text() == "")
	{	KMessageBox::sorry(this, i18n("<qt>You must enter a valid screen name</qt>"), i18n("Yahoo"));
		return false;
	}
	if(mPassword->text() == "")
	{	KMessageBox::sorry(this, i18n("<qt>You must enter a valid password</qt>"), i18n("Yahoo"));
		return false;
	}
	return true;
}

KopeteAccount *YahooEditAccount::apply()
{
	kdDebug(14180) << "YahooEditAccount::apply()";
	
	if(!m_account)
		m_account = new YahooAccount(theProtocol, mScreenName->text());
	else
		m_account->setAccountId(mScreenName->text());
	m_account->setAutoLogin(mAutoConnect->isChecked());
	if(mRememberPassword->isChecked())
		m_account->setPassword(mPassword->text());
	
	return m_account;
}

#include "yahooeditaccount.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

