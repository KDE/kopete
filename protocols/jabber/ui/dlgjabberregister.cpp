
/***************************************************************************
                          dlgjabberregister.cpp  -  description
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

#include "dlgjabberregister.h"
#include <qpushbutton.h>

#include <kmessagebox.h>
#include <klocale.h>

#include "jabberaccount.h"
#include "jabberprotocol.h"
#include "jabberclient.h"

dlgJabberRegister::dlgJabberRegister (JabberAccount *account, const XMPP::Jid & jid, QWidget * parent) : QWidget (parent)
{
	m_account = account;

	XMPP::JT_Register * task = new XMPP::JT_Register(m_account->client()->rootTask ());

	connect (task, SIGNAL (finished()), this, SLOT (slotGotForm()));

	task->getForm (jid);
	task->go (true);

	translator = 0;

}

void dlgJabberRegister::slotGotForm ()
{
	XMPP::JT_Register * task = (XMPP::JT_Register *) sender ();

	// remove the "wait" message
	delete lblWait;

	if (!task->success ())
	{
		KMessageBox::error (this, i18n ("Unable to retrieve registration form.\nReason: \"%1\"", task->statusString ()), i18n ("Jabber Error"));

		deleteLater ();

		return;
	}

	// translate the form and create it inside the box widget
	translator = new JabberFormTranslator (task->form (), grpForm);
	static_cast<QBoxLayout*>(grpForm->layout())->insertWidget(1, translator);
	translator->show();
	resize(sizeHint());

	// enable the send button
	btnRegister->setEnabled (true);

	connect (btnRegister, SIGNAL (clicked()), this, SLOT (slotSendForm()));

}

void dlgJabberRegister::slotSendForm ()
{
	if(!translator)
		return;
	XMPP::JT_Register * task = new XMPP::JT_Register (m_account->client()->rootTask ());

	connect (task, SIGNAL (finished()), this, SLOT (slotSentForm()));

	task->setForm (translator->resultData ());
	task->go (true);

	btnRegister->setEnabled (false);
	btnCancel->setEnabled (false);

}

void dlgJabberRegister::slotSentForm ()
{
	XMPP::JT_Register * task = (XMPP::JT_Register *) sender ();

	if (task->success ())
	{
		KMessageBox::information (this, i18n ("Registration sent successfully."), i18n ("Jabber Registration"));

		deleteLater ();
	}
	else
	{
		KMessageBox::error (this,
							i18n ("The server rejected the registration form.\nReason: \"%1\"", task->statusString ()), i18n ("Jabber Registration"));

		btnRegister->setEnabled (true);
		btnRegister->setEnabled (true);
	}

}

dlgJabberRegister::~dlgJabberRegister ()
{

	delete translator;

}

#include "dlgjabberregister.moc"
