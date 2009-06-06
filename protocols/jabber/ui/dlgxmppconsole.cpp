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

#include "dlgxmppconsole.h"

#include "jabberclient.h"

dlgXMPPConsole::dlgXMPPConsole(JabberClient *client, QWidget *parent):
KDialog(parent)
{
	mClient = client;
	setAttribute(Qt::WA_DeleteOnClose);
	QWidget *widget = new QWidget(this);
	ui.setupUi(widget);
	setMainWidget(widget);
	setCaption(i18n("XML Console"));
	// Buttons
	setButtons(Close | User1 | User2);
	setButtonText(User1, i18n("Clear"));
	setButtonText(User2, i18n("Send"));
	connect(this, SIGNAL(user1Clicked()), this, SLOT(slotClear()));
	connect(this, SIGNAL(user2Clicked()), this, SLOT(slotSend()));
}

dlgXMPPConsole::~dlgXMPPConsole()
{
}

void dlgXMPPConsole::slotIncomingXML(const QString &msg)
{
	ui.brLog->setTextColor(Qt::red);
	ui.brLog->append(msg);
}

void dlgXMPPConsole::slotOutgoingXML(const QString &msg)
{
	ui.brLog->setTextColor(Qt::blue);
	ui.brLog->append(msg);
}

void dlgXMPPConsole::slotSend()
{
	mClient->send(ui.mTextEdit->toPlainText());
}

void dlgXMPPConsole::slotClear()
{
	ui.brLog->clear();
}

#include "dlgxmppconsole.moc"
