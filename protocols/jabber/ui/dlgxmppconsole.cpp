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
QDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	ui.setupUi(this);
	mClient = client;

	connect(ui.btnSend, SIGNAL(clicked()), this, SLOT(slotSend()));
	connect(ui.btnClear, SIGNAL(clicked()), this, SLOT(slotClear()));
	connect(ui.btnClose, SIGNAL(clicked()), this, SLOT(close()));
}

dlgXMPPConsole::~dlgXMPPConsole()
{
}

void dlgXMPPConsole::slotIncomingXML(const QString &msg)
{
	ui.brLog->setColor(Qt::red);
	ui.brLog->append(msg);
}

void dlgXMPPConsole::slotOutgoingXML(const QString &msg)
{
	ui.brLog->setColor(Qt::blue);
	ui.brLog->append(msg);
}

void dlgXMPPConsole::slotSend()
{
	mClient->send(ui.mTextEdit->text());
}

void dlgXMPPConsole::slotClear()
{
	ui.brLog->clear();
}

#include "dlgxmppconsole.moc"
