
/***************************************************************************
                          dlgjabberbrowse.cpp  -  description
                             -------------------
    begin                : Wed Dec 11 2002
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

#include <kpushbutton.h>
#include <qgroupbox.h>
#include <qtable.h>
#include <qlabel.h>

#include <kmessagebox.h>
#include <klocale.h>

#include "jabberaccount.h"
#include "jabberprotocol.h"
#include "jabberclient.h"
#include "jabberformtranslator.h"
#include "dlgjabberbrowse.h"

dlgJabberBrowse::dlgJabberBrowse (JabberAccount *account, const XMPP::Jid & jid, QWidget * parent, const char *name):dlgBrowse (parent, name)
{
	m_account = account;

	// disable the left margin
	tblResults->setLeftMargin (0);

	// no content for now
	tblResults->setNumRows (0);

	// disable user selections
	tblResults->setSelectionMode (QTable::NoSelection);

	XMPP::JT_Search * task = new XMPP::JT_Search (m_account->client()->rootTask ());

	connect (task, SIGNAL (finished ()), this, SLOT (slotGotForm ()));

	task->get (jid);
	task->go (true);
}

void dlgJabberBrowse::slotGotForm ()
{
	XMPP::JT_Search * task = (XMPP::JT_Search *) sender ();

	// delete the wait message
	delete lblWait;

	if (!task->success ())
	{
		KMessageBox::queuedMessageBox(this, KMessageBox::Information, i18n ("Unable to retrieve search form."), i18n ("Jabber Error"));

		return;
	}


	// translate the form and create it inside the display widget
	translator = new JabberFormTranslator (task->form (), dynamicForm);
	dynamicForm->layout()->add( translator );
	translator->show();

	// enable the send button
	btnSearch->setEnabled (true);

	// adjust table
	tblResults->setNumCols (5);

	for (int i = 0; i < 5; i++)
	{
		// allow autostretching
		tblResults->setColumnStretchable (i, true);
	}

	connect (btnSearch, SIGNAL (clicked ()), this, SLOT (slotSendForm ()));

}

void dlgJabberBrowse::slotSendForm ()
{

	XMPP::JT_Search * task = new XMPP::JT_Search (m_account->client()->rootTask ());

	connect (task, SIGNAL (finished ()), this, SLOT (slotSentForm ()));

	task->set (translator->resultData ());
	task->go (true);

	btnSearch->setEnabled (false);
	btnClose->setEnabled (false);

}

void dlgJabberBrowse::slotSentForm ()
{
	XMPP::JT_Search * task = (XMPP::JT_Search *) sender ();

	btnSearch->setEnabled (true);
	btnClose->setEnabled (true);

	if (!task->success ())
	{
		KMessageBox::queuedMessageBox (this, KMessageBox::Error, i18n ("The Jabber server declined the search."), i18n ("Jabber Search"));

		return;
	}

	tblResults->setNumRows (task->results ().count ());

	int row = 0;

	for (QValueList < XMPP::SearchResult >::const_iterator it = task->results ().begin (); it != task->results ().end (); ++it)
	{
		tblResults->setText (row, 0, (*it).jid ().userHost ());
		tblResults->setText (row, 1, (*it).first ());
		tblResults->setText (row, 2, (*it).last ());
		tblResults->setText (row, 3, (*it).nick ());
		tblResults->setText (row, 4, (*it).email ());

		row++;
	}
	for (int i = 0; i < 5; i++)
	{
		tblResults->setColumnStretchable (i, false);
		tblResults->adjustColumn (i);
	}
}

dlgJabberBrowse::~dlgJabberBrowse ()
{
}

#include "dlgjabberbrowse.moc"
