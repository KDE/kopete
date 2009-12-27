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

#include "dlgahcommand.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <KLocale>
#include <KDebug>

#include "jt_ahcommand.h"
#include "jabberxdatawidget.h"

dlgAHCommand::dlgAHCommand(const AHCommand &r, const XMPP::Jid &jid, XMPP::Client *client, bool final, QWidget *parent):
KDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	mNode = r.node();
	mSessionId = r.sessionId();
	mJid = jid;
	mClient = client;
	// XData form
	mXDataWidget = new JabberXDataWidget(r.data(), this);
	setMainWidget(mXDataWidget);
	if (!r.data().title().isEmpty())
		setCaption(r.data().title());
	else
		setCaption(i18n("Command executing"));

	// Buttons
	if(!final)
	{
		if(r.actions().empty())
		{
			setButtons(Ok | Cancel);
			setButtonText(Ok, i18n("Finish"));
			connect(this, SIGNAL(okClicked()), SLOT(slotExecute()));
		}
		else
		{
			setButtons(Ok | Cancel | User1 | User2);
			setButtonText(User1, i18n("Next"));
			setButtonText(User2, i18n("Previous"));
			setButtonText(Ok, i18n("Finish"));
			// Multi-stage dialog
			// Previous
			if(r.actions().contains(AHCommand::Prev))
			{
				if(r.defaultAction() == AHCommand::Prev)
					setDefaultButton(User2);
				connect(this, SIGNAL(user2Clicked()), SLOT(slotPrev()));
				enableButton(User2, true);
			}
			else
				enableButton(User2, false);
			// Next
			if(r.actions().contains(AHCommand::Next))
			{
				if(r.defaultAction() == AHCommand::Next)
				{
					connect(this, SIGNAL(user1Clicked()), SLOT(slotExecute()));
					setDefaultButton(User1);
				}
				else
					connect(this, SIGNAL(user1Clicked()), SLOT(slotNext()));

				enableButton(User1, true);
			}
			else
				enableButton(User1, false);
			// Complete
			if(r.actions().contains(AHCommand::Complete))
			{
				if(r.defaultAction() == AHCommand::Complete)
				{
					connect(this, SIGNAL(okClicked()), SLOT(slotExecute()));
					setDefaultButton(Ok);
				}
				else
					connect(this, SIGNAL(okClicked()), SLOT(slotComplete()));
				enableButton(Ok, true);
			}
			else
				enableButton(Ok, false);
		}
		connect(this, SIGNAL(cancelClicked()), SLOT(slotCancel()));
	}
	else
	{
		setButtons(Ok);
	}
}

dlgAHCommand::~dlgAHCommand()
{

}

void dlgAHCommand::slotPrev()
{
	JT_AHCommand *task = new JT_AHCommand(mJid, AHCommand(mNode, data(), mSessionId, AHCommand::Prev), mClient->rootTask());
	connect(task, SIGNAL(finished()), SLOT(close()));
	task->go(true);
}

void dlgAHCommand::slotNext()
{
	JT_AHCommand *task = new JT_AHCommand(mJid, AHCommand(mNode, data(), mSessionId, AHCommand::Next), mClient->rootTask());
	connect(task, SIGNAL(finished()), SLOT(close()));
	task->go(true);
}

void dlgAHCommand::slotComplete()
{
	JT_AHCommand *task = new JT_AHCommand(mJid, AHCommand(mNode, data(), mSessionId, AHCommand::Complete), mClient->rootTask());
	connect(task, SIGNAL(finished()), SLOT(close()));
	task->go(true);
}

void dlgAHCommand::slotExecute()
{
	JT_AHCommand *task = new JT_AHCommand(mJid, AHCommand(mNode, data(), mSessionId), mClient->rootTask());
	connect(task, SIGNAL(finished()), SLOT(close()));
	task->go(true);
}

void dlgAHCommand::slotCancel()
{
	JT_AHCommand *task = new JT_AHCommand(mJid, AHCommand(mNode, data(), mSessionId, AHCommand::Cancel), mClient->rootTask());
	connect(task, SIGNAL(finished()), SLOT(close()));
	task->go(true);
}

XMPP::XData dlgAHCommand::data() const
{
	XMPP::XData x;
	x.setFields(mXDataWidget->fields());
	x.setType(XMPP::XData::Data_Submit);
	return x;
}

#include "dlgahcommand.moc"
