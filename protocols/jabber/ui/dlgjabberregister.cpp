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

#include <kmessagebox.h>
#include <klocale.h>
 
#include <psi/types.h>
#include <psi/tasks.h>

#include "jabberprotocol.h"
#include "jabberformtranslator.h"
#include "dlgjabberregister.h"

DlgJabberRegister::DlgJabberRegister(const Jabber::Jid &jid, QWidget *parent, const char *name ) : dlgRegister(parent,name)
{

	Jabber::JT_Register *task = new Jabber::JT_Register(JabberProtocol::protocol()->jabberClient->rootTask());

	connect(task, SIGNAL(finished()), this, SLOT(slotGotForm()));

	task->getForm(jid);
	task->go(true);

}

void DlgJabberRegister::slotGotForm()
{
	Jabber::JT_Register *task = (Jabber::JT_Register *)sender();

	// remove the "wait" message
	delete lblWait;

	if(!task->success())
	{
		KMessageBox::information(qApp->mainWidget(),
								 i18n("Unable to retrieve registration form"),
								 i18n("Jabber Error"));

		return;
	}

	QGridLayout *layout = new QGridLayout(grpForm, 1, 1, 20, 10);
	
	JabberFormTranslator::translate(task->form(), layout, grpForm);

}

DlgJabberRegister::~DlgJabberRegister()
{
}
