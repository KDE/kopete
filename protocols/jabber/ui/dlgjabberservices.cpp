/***************************************************************************
                          dlgjabberservices.cpp  -  description
                             -------------------
    begin                : Mon Dec 9 2002
    copyright            : (C) 2002 by Till Gerken
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
#include "dlgjabberregister.h"
#include "dlgjabberbrowse.h"
#include "dlgjabberservices.h"

#include "dlgjabberservices.moc"

DlgJabberServices::DlgJabberServices(QWidget *parent, const char *name ) : dlgServices(parent,name)
{

	if(JabberProtocol::protocol()->isConnected())
	{
		// pre-populate the server field
		leServer->setText(Jid(JabberProtocol::protocol()->myContact->contactId()).host());
	}

	// disable the left margin
	tblServices->setLeftMargin(0);

	// no content for now
	tblServices->setNumRows(0);

	// disable the buttons as long as nothing has been selected
	btnRegister->setDisabled(true);
	btnBrowse->setDisabled(true);

	// allow autostretching
	tblServices->setColumnStretchable(0, true);
	tblServices->setColumnStretchable(1, true);

	// disable user selections
	tblServices->setSelectionMode(QTable::NoSelection);
	
	// name table headers
	tblServices->horizontalHeader()->setLabel(0, i18n("Name"));
	tblServices->horizontalHeader()->setLabel(1, i18n("Address"));
	
	connect(btnQuery, SIGNAL(clicked()), this, SLOT(slotQuery()));
	connect(tblServices, SIGNAL(clicked(int, int, int, const QPoint &)), this, SLOT(slotSetSelection(int, int, int, const QPoint &)));

	connect(btnRegister, SIGNAL(clicked()), this, SLOT(slotRegister()));
	connect(btnBrowse, SIGNAL(clicked()), this, SLOT(slotBrowse()));

	serviceTask = 0L;
	
	selectedRow = 0;

}

void DlgJabberServices::slotSetSelection(int row, int, int, const QPoint &)
{

	tblServices->clearSelection(true);
#if QT_VERSION >= 0x030100
	tblServices->addSelection(QTableSelection(row, 0, row, 1));
#else
	QTableSelection *selection = new QTableSelection();
	selection->init(row, 0);
	selection->expandTo(row, 1);
	tblServices->addSelection(*selection);
#endif

	// query the agent list about the selected item
	btnRegister->setDisabled(!serviceTask->agents()[row].canRegister());
	btnBrowse->setDisabled(!serviceTask->agents()[row].canSearch());

	selectedRow = row;
	
}

void DlgJabberServices::slotQuery()
{

	if(!JabberProtocol::protocol()->isConnected())
	{
		JabberProtocol::protocol()->errorConnectFirst();
		return;
	}

	// create the jabber task
	delete serviceTask;
	serviceTask = new Jabber::JT_GetServices(JabberProtocol::protocol()->jabberClient->rootTask());
	connect(serviceTask, SIGNAL(finished()), this, SLOT(slotQueryFinished()));

	/* populate server field if it is empty */
	if(leServer->text().isEmpty())
		leServer->setText(Jid(JabberProtocol::protocol()->myContact->contactId()).host());

	kdDebug(14130) << "[DlgJabberServices] Trying to fetch a list of services at " << leServer->text() << endl;

	serviceTask->get(leServer->text());
	serviceTask->go(false);

}

void DlgJabberServices::slotQueryFinished()
{
	kdDebug(14130) << "[DlgJabberServices] Query task finished" << endl;

	Jabber::JT_GetServices *task = (Jabber::JT_GetServices *)sender();

	if(!task->success())
	{
		KMessageBox::error(this,
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

void DlgJabberServices::slotRegister()
{

	DlgJabberRegister *registerDialog = new DlgJabberRegister(serviceTask->agents()[selectedRow].jid());

	registerDialog->show();
	registerDialog->raise();

}

void DlgJabberServices::slotBrowse()
{

	DlgJabberBrowse *browseDialog = new DlgJabberBrowse(serviceTask->agents()[selectedRow].jid());

	browseDialog->show();
	browseDialog->raise();

}

DlgJabberServices::~DlgJabberServices()
{

	delete serviceTask;

}
