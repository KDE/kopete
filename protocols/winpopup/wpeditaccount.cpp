/***************************************************************************
                          wpeditaccount.cpp  -  description
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

// Standard Unix Includes
#include <unistd.h>

// QT Includes
#include <qcheckbox.h>
#include <qfile.h>

// KDE Includes
#include <kdebug.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <knuminput.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kapplication.h>

// Kopete Includes
#include <addcontactpage.h>

// Local Includes
#include "wpaccount.h"
#include "wpeditaccount.h"

WPEditAccount::WPEditAccount(WPProtocol *protocol, Kopete::Account *theAccount, QWidget *parent, const char *name)
	: WPEditAccountBase(parent), KopeteEditAccountWidget(theAccount), mProtocol(protocol)
{
	kdDebug(14170) << "WPEditAccount::WPEditAccount(<protocol>, <theAccount>, <parent>, " << name << ")";

	if(account()) {
		mHostName->setText(account()->accountId());
		mAutoConnect->setChecked(account()->excludeConnect());
		mHostName->setReadOnly(true);
		KGlobal::config()->setGroup("WinPopup");
		mMessageCheckFreq->setValue(KGlobal::config()->readNumEntry("MessageCheckFreq", 5));
		mHostCheckFreq->setValue(KGlobal::config()->readNumEntry("HostCheckFreq", 60));
		mSmbcPath->setURL(KGlobal::config()->readEntry("SmbcPath", "/usr/bin/smbclient"));

	}
	else {
		// no QT/KDE function? GF
		QString theHostName = QString::null;
		char *tmp = new char[255];

		if (tmp != 0) {
			gethostname(tmp, 255);
			theHostName = tmp;
			if (theHostName.contains('.') != 0) theHostName.remove(theHostName.find('.'), theHostName.length());
			theHostName = theHostName.toUpper();
		}

		if (!theHostName.isEmpty())
			mHostName->setText(theHostName);
		else
			mHostName->setText("LOCALHOST");

		mMessageCheckFreq->setValue(5);
		mHostCheckFreq->setValue(60);
		mSmbcPath->setURL("/usr/bin/smbclient");
	}

	show();
}

void WPEditAccount::installSamba()
{
	mProtocol->installSamba();
}

bool WPEditAccount::validateData()
{
	kdDebug(14170) << "WPEditAccount::validateData()";

	if(mHostName->text().isEmpty()) {
		KMessageBox::sorry(this, i18n("<qt>You must enter a valid screen name.</qt>"), i18n("WinPopup"));
		return false;
	}

	QFile smbc(mSmbcPath->url());
	if (!smbc.exists()) {
		KMessageBox::sorry(this, i18n("<qt>You must enter a valid smbclient path.</qt>"), i18n("WinPopup"));
		return false;
	}

	if (!mProtocol->checkMessageDir()) {
		KMessageBox::sorry(this, i18n("<qt>There is a serious problem with your working directory.</qt>"), i18n("WinPopup"));
		return false;
	}

	return true;
}

void WPEditAccount::writeConfig()
{
	KGlobal::config()->setGroup("WinPopup");
	KGlobal::config()->writeEntry("SmbcPath", mSmbcPath->url());
	KGlobal::config()->writeEntry("HostCheckFreq", mHostCheckFreq->text());
	KGlobal::config()->writeEntry("MessageCheckFreq", mMessageCheckFreq->text());
}

Kopete::Account *WPEditAccount::apply()
{
	kdDebug(14170) << "WPEditAccount::apply()";

	if(!account())
		setAccount(new WPAccount(mProtocol, mHostName->text()));

	account()->setExcludeConnect(mAutoConnect->isChecked());
	writeConfig();

	//is there already an API function or signal? GF
	dynamic_cast<WPAccount *>(account())->slotSettingsChanged();

	return account();
}

#include "wpeditaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
