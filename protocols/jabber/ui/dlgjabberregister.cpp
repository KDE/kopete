/***************************************************************************
                          dlgjabberregister.cpp  -  description
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

#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qpushbutton.h>

#include <kmessagebox.h>
#include <klocale.h>
 
#include "types.h"
#include "tasks.h"

#include "jabberprotocol.h"
#include "jabberformtranslator.h"
#include "dlgjabberregister.h"

DlgJabberRegister::DlgJabberRegister(const Jabber::Jid &jid, QWidget *parent, const char *name ) : dlgRegister(parent,name)
{

	Jabber::JT_Register *task = new Jabber::JT_Register(JabberProtocol::protocol()->jabberClient->rootTask());

	connect(task, SIGNAL(finished()), this, SLOT(slotGotForm()));

	task->getForm(jid);
	task->go(true);

	translator = 0;

}

void DlgJabberRegister::slotGotForm()
{
	Jabber::JT_Register *task = (Jabber::JT_Register *)sender();

	// remove the "wait" message
	delete lblWait;

	if(!task->success())
	{
		KMessageBox::error(this,
								 i18n("Unable to retrieve registration form.\nReason: \"%1\"").arg(task->statusString(), 1),
								 i18n("Jabber Error"));

		deleteLater();
		
		return;
	}

	// translate the form and create it inside the box widget
	translator = new JabberFormTranslator(grpForm);

	translator->translate(task->form(), grpForm);

	// enable the send button
	btnRegister->setEnabled(true);

	connect(btnRegister, SIGNAL(clicked()), this, SLOT(slotSendForm()));

}

void DlgJabberRegister::slotSendForm()
{

	Jabber::JT_Register *task = new Jabber::JT_Register(JabberProtocol::protocol()->jabberClient->rootTask());

	connect(task, SIGNAL(finished()), this, SLOT(slotSentForm()));
	
	task->setForm(translator->resultData());
	task->go(true);

	btnRegister->setEnabled(false);
	btnCancel->setEnabled(false);

}

void DlgJabberRegister::slotSentForm()
{
	Jabber::JT_Register *task = (Jabber::JT_Register *)sender();

	if(task->success())
	{
		KMessageBox::information(this,
								 i18n("Registration sent successfully"),
								 i18n("Jabber Registration"));

		deleteLater();
	}
	else
	{
		KMessageBox::error(this,
								 i18n("The server denied the registration form.\nReason: \"%1\"").arg(task->statusString(), 1),
								 i18n("Jabber Registration"));

		btnRegister->setEnabled(true);
		btnRegister->setEnabled(true);
	}

}

DlgJabberRegister::~DlgJabberRegister()
{

	delete translator;

}
#include "dlgjabberregister.moc"
