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

#include "jt_ahcommand.h"
#include "jabberxdatawidget.h"

dlgAHCommand::dlgAHCommand(const AHCommand &r, const XMPP::Jid &jid, XMPP::Client *client, bool final, QWidget *parent):
QDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	mNode = r.node();
	mSessionId = r.sessionId();
	mJid = jid;
	mClient = client;
	QVBoxLayout *vb = new QVBoxLayout(this, 11, 6);
	// XData form
	mXDataWidget = new JabberXDataWidget(r.data(), this);
	vb->addWidget(mXDataWidget);

	// Buttons
	QHBoxLayout *buttons = new QHBoxLayout(vb);
	if(!final)
	{
		if(r.actions().empty())
		{
			btnComplete = new QPushButton("Finish", this);
			connect(btnComplete, SIGNAL(clicked()), SLOT(slotExecute()));
			buttons->addWidget(btnComplete);
		}
		else
		{
			// Multi-stage dialog
			// Previous
			btnPrev = new QPushButton("Previous", this);
			buttons->addWidget(btnPrev);
			if(r.actions().contains(AHCommand::Prev))
			{
				if(r.defaultAction() == AHCommand::Prev)
				{
					btnPrev->setDefault(true);
					btnPrev->setFocus();
				}
				connect(btnPrev, SIGNAL(clicked()), SLOT(slotPrev()));
				btnPrev->setEnabled(true);
			}
			else 
				btnPrev->setEnabled(false);
			// Next
			btnNext = new QPushButton("Next", this);
			buttons->addWidget(btnNext);
			if(r.actions().contains(AHCommand::Next))
			{
				if(r.defaultAction() == AHCommand::Next)
				{
					connect(btnNext, SIGNAL(clicked()), SLOT(slotExecute()));
					btnNext->setDefault(true);
					btnNext->setFocus();
				}
				else
					connect(btnNext, SIGNAL(clicked()), SLOT(slotNext()));
				btnNext->setEnabled(true);
			}
			else
				btnNext->setEnabled(false);
			// Complete
			btnComplete = new QPushButton("Finish", this);
			buttons->addWidget(btnComplete);
			if(r.actions().contains(AHCommand::Complete))
			{
				if(r.defaultAction() == AHCommand::Complete)
				{
					connect(btnComplete, SIGNAL(clicked()), SLOT(slotExecute()));
					btnComplete->setDefault(true);
					btnComplete->setFocus();
				}
				else
					connect(btnComplete, SIGNAL(clicked()), SLOT(slotComplete()));
				btnComplete->setEnabled(true);
			}
			else
				btnComplete->setEnabled(false);
		}
		btnCancel = new QPushButton("Cancel", this);
		connect(btnCancel, SIGNAL(clicked()), SLOT(slotCancel()));
		buttons->addWidget(btnCancel);
	}
	else
	{
		QPushButton *btnComplete = new QPushButton("Ok", this);
		connect(btnComplete, SIGNAL(clicked()), SLOT(close()));
		buttons->addWidget(btnComplete);
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
