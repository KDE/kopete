/***************************************************************************
                          dlgjabberservices.cpp  -  description
                             -------------------
    begin                : Mon Dec 9 2002
    copyright            : (C) 2002 by Kopete developers
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <qapplication.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qtable.h>

#include <psi/types.h>
#include <psi/tasks.h>

#include "jabbercontact.h"
#include "jabberprotocol.h"
#include "dlgjabberservices.h"
#include "dlgjabberservices.moc"

DlgJabberServices::DlgJabberServices(QWidget *parent, const char *name ) : dlgServices(parent,name)
{

	if(JabberProtocol::protocol()->isConnected())
	{
		// pre-populate the server field
		leServer->setText(Jid(JabberProtocol::protocol()->myContact->id()).host());
	}

	// disable the left margin
	tblServices->setLeftMargin(0);

	// disable the "register" button while nothing has been selected
	btnRegister->setDisabled(true);

	// allow autostretching
	tblServices->setColumnStretchable(0, true);
	tblServices->setColumnStretchable(1, true);
	
	// name table headers
	tblServices->horizontalHeader()->setLabel(0, i18n("Name"));
	tblServices->horizontalHeader()->setLabel(1, i18n("Address"));
	
	connect(btnQuery, SIGNAL(clicked()), this, SLOT(slotQuery()));

}

void DlgJabberServices::slotQuery()
{

	if(!JabberProtocol::protocol()->isConnected())
	{
		JabberProtocol::protocol()->errorConnectFirst();
		return;
	}

	/* populate server field if it is empty */
	if(leServer->text().isEmpty())
		leServer->setText(Jid(JabberProtocol::protocol()->myContact->id()).host());

	kdDebug() << "[DlgJabberServices] Trying to fetch a list of services at " << leServer->text() << endl;

	Jabber::JT_GetServices *serviceTask = new Jabber::JT_GetServices(JabberProtocol::protocol()->jabberClient->rootTask());
	connect(serviceTask, SIGNAL(finished()), this, SLOT(slotQueryFinished()));
	serviceTask->get(leServer->text());
	serviceTask->go(true);

}

void DlgJabberServices::slotQueryFinished()
{
	kdDebug() << "[DlgJabberServices] Query task finished" << endl;

	Jabber::JT_GetServices *task = (Jabber::JT_GetServices *)sender();

	if(!task->success())
	{
		KMessageBox::information(qApp->mainWidget(),
								 i18n("Unable to retrieve the list of services"),
								 i18n("Jabber Error"));
		return;
	}

	tblServices->setNumRows(task->agents().count());
	
	int row = 0;
	for(Jabber::AgentList::const_iterator it = task->agents().begin(); it != task->agents().end(); it++)
	{
		tblServices->setText(row, 0, (*it).name());
		tblServices->setText(row, 1, (*it).jid().userHost());
		row++;
	}

}

DlgJabberServices::~DlgJabberServices()
{
}
