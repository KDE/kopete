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
#include <qcombobox.h>
#include <qradiobutton.h>

#include "ircaddcontactpage.h"
#include "ircadd.h"
#include "ircprotocol.h"
#include "kopetecontactlist.h"

IRCAddContactPage::IRCAddContactPage(IRCProtocol *owner, QWidget *parent, const char *name )
				  : AddContactPage(parent,name)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	ircdata = new ircAddUI(this);
	plugin = owner;

	KGlobal::config()->setGroup("IRC");
	QString server = KGlobal::config()->readEntry("Server", "irc.freenode.net");
	ircdata->ircServer->lineEdit()->setText(server);
}
IRCAddContactPage::~IRCAddContactPage()
{
}

void IRCAddContactPage::slotFinish(KopeteMetaContact *m)
{
	QString server = ircdata->ircServer->lineEdit()->text();
	QString name = ircdata->addID->text();
	plugin->addContact(server, name, ircdata->rdoChannel->isChecked() ? true : false, m);
}

bool IRCAddContactPage::validateData()
{
	QString server = ircdata->ircServer->lineEdit()->text();
	if (server.isEmpty() == true)
	{
		KMessageBox::sorry(this, i18n("<qt>You need to specify a server to connect to.</qt>"), i18n("You Must Specify a Server"));
		return false;
	}
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

