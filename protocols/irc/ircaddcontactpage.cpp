/*
    ircaddcontactpage.cpp - IRC Add Contact Widget

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

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

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include "ircaddcontactpage.h"
#include "ircadd.h"
#include "ircprotocol.h"
#include "ircaccount.h"
#include "kopetecontactlist.h"

IRCAddContactPage::IRCAddContactPage( QWidget *parent, IRCAccount * ) : AddContactPage(parent, 0)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	ircdata = new ircAddUI(this);
	ircdata->searchResults->setEnabled( false );
	ircdata->searchButton->setEnabled( false );
}
IRCAddContactPage::~IRCAddContactPage()
{
}

bool IRCAddContactPage::apply(KopeteAccount *account , KopeteMetaContact *m)
{
	QString name = ircdata->addID->text();
	static_cast<IRCAccount*>(account)->addContact(name, name, m);
	return true;
}

bool IRCAddContactPage::validateData()
{
	QString name = ircdata->addID->text();
	if (name.isEmpty() == true)
	{
		KMessageBox::sorry(this, i18n("<qt>You need to specify a channel to join, or query to open.</qt>"), i18n("You Must Specify a Channel"));
		return false;
	}
	return true;
}

#include "ircaddcontactpage.moc"

// vim: set noet ts=4 sts=4 sw=4:

