 /*
    Copyright (c) 2008 by Igor Janssen  <alaves17@gmail.com>

    Kopete    (c) 2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#include "dlgahclist.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <KLocale>
#include <KDebug>

#include "tasks/jt_ahcommand.h"

dlgAHCList::dlgAHCList(const XMPP::Jid &jid, XMPP::Client *client, QWidget *parent):
KDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	mJid = jid;
	mClient = client;
	mCommandsWidget = new QWidget(this);
	setMainWidget(mCommandsWidget);
	mCommandsLayout = 0L;
	setButtons(Close | User1 | User2);
	setButtonText(User1, i18n("Execute"));
	setButtonText(User2, i18n("Refresh"));
	setCaption(i18n("Execute command"));
	connect(this, SIGNAL(user1Clicked()), SLOT(slotExecuteCommand()));
	connect(this, SIGNAL(user2Clicked()), SLOT(slotGetList()));
	slotGetList();
}

dlgAHCList::~dlgAHCList()
{
}

void dlgAHCList::slotGetList()
{
	if(mCommandsLayout)
		delete mCommandsLayout;
	foreach(const Item &item, mCommands)
		delete item.radio;
	mCommands.clear();
	JT_AHCGetList *t = new JT_AHCGetList(mClient->rootTask(), mJid);
	connect(t, SIGNAL(finished()), SLOT(slotListReceived()));
	t->go(true);
}

void dlgAHCList::slotListReceived()
{
	JT_AHCGetList *t = (JT_AHCGetList *)sender();
	Item item;
	mCommandsLayout = new QVBoxLayout(mCommandsWidget);
	foreach(const JT_AHCGetList::Item &i, t->commands())
	{
		item.radio = new QRadioButton(i.name, mCommandsWidget);
		mCommandsLayout->addWidget(item.radio);
		item.jid = i.jid;
		item.node = i.node;
		mCommands.append(item);
	}
	mCommandsLayout->addStretch(1);
	if(mCommands.count() > 0)
		mCommands[0].radio->setChecked(true);
}

void dlgAHCList::slotExecuteCommand()
{
	foreach(const Item &item, mCommands)
		if(item.radio->isChecked())
		{
			JT_AHCommand *t = new JT_AHCommand(item.jid, AHCommand(item.node), mClient->rootTask());
			connect(t, SIGNAL(finished()), SLOT(slotCommandExecuted()));
			t->go(true);
			break;
		}
}

void dlgAHCList::slotCommandExecuted()
{
	close();
}

#include "dlgahclist.moc"
