
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

#include "dlgregister.h"

#include <qpushbutton.h>
//Added by qt3to4:
#include <Q3BoxLayout>

#include <KMessageBox>
#include <KLocale>

#include "jabberaccount.h"
#include "jabberprotocol.h"
#include "jabberclient.h"
#include "jabberxdatawidget.h"
#include "jt_xregister.h"

dlgRegister::dlgRegister(JabberAccount *account, const XMPP::Jid &jid, QWidget *parent):
QDialog(parent)
{
	setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	m_account = account;
	mXDataWidget = 0L;
	translator = 0;

	lblWait->setText(i18n("Please wait while querying the server..."));
	connect (btnRegister, SIGNAL(clicked()), this, SLOT(slotSendForm()));
	connect(btnCancel, SIGNAL(clicked()), SLOT(close()));

	JT_XRegister *task = new JT_XRegister(m_account->client()->rootTask());
	connect (task, SIGNAL (finished ()), this, SLOT (slotGotForm ()));
	task->getForm(jid);
	task->go(true);
}

dlgRegister::~dlgRegister()
{
	delete translator;
}

void dlgRegister::slotGotForm()
{
	JT_XRegister *task = (JT_XRegister *)sender();

	// remove the "wait" message
	delete lblWait;

	if(!task->success())
	{
		KMessageBox::error(this, i18n("Unable to retrieve registration form.\nReason: \"%1\"", task->statusString()), i18n("Jabber Error"));
		deleteLater();
		return;
	}

	mForm = task->form();
	QDomElement e = task->xdataElement();
	if(!e.isNull())
	{
		XMPP::XData form;
		form.fromXml(e);
		mXDataWidget = new JabberXDataWidget(form, grpForm);
		grpForm->layout()->addWidget(mXDataWidget);
		mXDataWidget->show();
	}
	else
	{
		// translate the form and create it inside the box widget
		translator = new JabberFormTranslator(mForm, grpForm);
		static_cast<Q3BoxLayout*>(grpForm->layout())->insertWidget(1, translator);
		translator->show();
	}
	resize(sizeHint());

	// enable the send button
	btnRegister->setEnabled(true);
}

void dlgRegister::slotSendForm ()
{
	if(!translator && !mXDataWidget)
		return;
	JT_XRegister *task = new JT_XRegister(m_account->client()->rootTask());
	connect(task, SIGNAL(finished()), this, SLOT(slotSentForm()));

	if(mXDataWidget)
	{
		XMPP::XData form;
		form.setFields(mXDataWidget->fields());
		task->setXForm(mForm, form);
	}
	else
	{
		task->setForm(translator->resultData());
	}
	task->go(true);

	btnRegister->setEnabled(false);
	btnCancel->setEnabled(false);

}

void dlgRegister::slotSentForm ()
{
	JT_XRegister *task = (JT_XRegister *)sender();
	if (task->success ())
	{
		KMessageBox::information(this, i18n("Registration sent successfully."), i18n("Jabber Registration"));
		deleteLater();
	}
	else
	{
		KMessageBox::error(this, i18n("The server rejected the registration form.\nReason: \"%1\"", task->statusString()), i18n("Jabber Registration"));
		btnRegister->setEnabled(true);
	}
}

#include "dlgregister.moc"
