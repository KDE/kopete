
/***************************************************************************
                          dlgjabberservices.cpp  -  Service browsing
                             -------------------
    begin                : Mon Dec 9 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
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

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qtable.h>


#include "jabberaccount.h"
#include "dlgjabberregister.h"
#include "dlgjabberbrowse.h"
#include "dlgjabberservices.h"

#include "dlgjabberservices.moc"

dlgJabberServices::dlgJabberServices (JabberAccount *account, QWidget *parent, const char *name):dlgServices (parent, name)
{
	m_account = account;

	if(m_account->isConnected())
	{
		// pre-populate the server field
		leServer->setText(m_account->server());
	}

	// disable the left margin
	tblServices->setLeftMargin (0);

	// no content for now
	tblServices->setNumRows (0);

	// disable the buttons as long as nothing has been selected
	btnRegister->setDisabled (true);
	btnBrowse->setDisabled (true);

	// allow autostretching
	tblServices->setColumnStretchable (0, true);
	tblServices->setColumnStretchable (1, true);

	// disable user selections
	tblServices->setSelectionMode (QTable::NoSelection);

	// name table headers
	tblServices->horizontalHeader ()->setLabel (0, i18n ("Name"));
	tblServices->horizontalHeader ()->setLabel (1, i18n ("Address"));

	connect (btnQuery, SIGNAL (clicked ()), this, SLOT (slotQuery ()));
	connect (tblServices, SIGNAL (clicked (int, int, int, const QPoint &)), this, SLOT (slotSetSelection (int, int, int, const QPoint &)));

	connect (btnRegister, SIGNAL (clicked ()), this, SLOT (slotRegister ()));
	connect (btnBrowse, SIGNAL (clicked ()), this, SLOT (slotBrowse ()));

	serviceTask = 0L;

	selectedRow = 0;

}

void dlgJabberServices::slotSetSelection (int row, int, int, const QPoint &)
{

	if(serviceTask && (uint(row) <= serviceTask->agents().count()))
	{
		tblServices->clearSelection (true);
		tblServices->addSelection (QTableSelection (row, 0, row, 1));

		// query the agent list about the selected item
		btnRegister->setDisabled (!serviceTask->agents()[row].features().canRegister ());
		btnBrowse->setDisabled (!serviceTask->agents()[row].features().canSearch ());

		selectedRow = row;
	}

}

void dlgJabberServices::slotQuery ()
{

	if(!m_account->isConnected())
	{
		m_account->errorConnectFirst();
		return;
	}

	// create the jabber task
	delete serviceTask;

	serviceTask = new XMPP::JT_GetServices (m_account->client()->rootTask ());
	connect (serviceTask, SIGNAL (finished ()), this, SLOT (slotQueryFinished ()));

	/* populate server field if it is empty */
	if(leServer->text().isEmpty())
		leServer->setText(m_account->server());

	kdDebug (14130) << "[dlgJabberServices] Trying to fetch a list of services at " << leServer->text () << endl;

	serviceTask->get (leServer->text ());
	serviceTask->go (false);

}

void dlgJabberServices::slotQueryFinished ()
{
	kdDebug (14130) << "[dlgJabberServices] Query task finished" << endl;

	XMPP::JT_GetServices * task = (XMPP::JT_GetServices *) sender ();

	if (!task->success ())
	{
		KMessageBox::error (this, i18n ("Unable to retrieve the list of services."), i18n ("Jabber Error"));
		return;
	}

	tblServices->setNumRows (task->agents ().count ());

	int row = 0;

	for (XMPP::AgentList::const_iterator it = task->agents ().begin (); it != task->agents ().end (); ++it)
	{
		tblServices->setText (row, 0, (*it).name ());
		tblServices->setText (row, 1, (*it).jid ().userHost ());
		row++;
	}

}

void dlgJabberServices::slotRegister ()
{

	dlgJabberRegister *registerDialog = new dlgJabberRegister (m_account, serviceTask->agents ()[selectedRow].jid ());

	registerDialog->show ();
	registerDialog->raise ();

}

void dlgJabberServices::slotBrowse ()
{

	dlgJabberBrowse *browseDialog = new dlgJabberBrowse (m_account, serviceTask->agents ()[selectedRow].jid ());

	browseDialog->show ();
	browseDialog->raise ();

}

dlgJabberServices::~dlgJabberServices ()
{

	delete serviceTask;

}
