
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

#include <QLabel>
#include <KMessageBox>
#include <KLocale>
#include <KDebug>

#include "jabberaccount.h"
#include "jabberprotocol.h"
#include "jabberclient.h"
#include "jabberformtranslator.h"
#include "jabberxdatawidget.h"
#include "jt_xregister.h"

dlgRegister::dlgRegister(JabberAccount *account, const XMPP::Jid &jid, QWidget *parent):
KDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	mAccount = account;
	mXDataWidget = 0L;
	mTranslator = 0L;

	mMainWidget = new QWidget(this);
	setMainWidget(mMainWidget);
	lblWait = new QLabel(mMainWidget);
	lblWait->setText(i18n("Please wait while querying the server..."));
	QVBoxLayout *layout = new QVBoxLayout(mMainWidget);
	layout->addWidget(lblWait);
	setCaption(i18n("Register"));

	setButtons(Close | User1);
	setButtonText(User1, i18n("Register"));
	connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSendForm()));

	JT_XRegister *task = new JT_XRegister(mAccount->client()->rootTask());
	connect (task, SIGNAL (finished()), this, SLOT (slotGotForm()));
	task->getForm(jid);
	task->go(true);
}

dlgRegister::~dlgRegister()
{
	delete mTranslator;
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
		mXDataWidget = new JabberXDataWidget(form, mMainWidget);
		//kDebug() << "COUNT " << mMainWidget->layout()->count();
		mMainWidget->layout()->addWidget(mXDataWidget);
		mXDataWidget->show();
	}
	else
	{
		// translate the form and create it inside the box widget
		mTranslator = new JabberFormTranslator(mForm, mMainWidget);
		mMainWidget->layout()->addWidget(mTranslator);
		mTranslator->show();
	}
	resize(sizeHint());

	// enable the send button
	//btnRegister->setEnabled(true);
}

void dlgRegister::slotSendForm()
{
	if(!mTranslator && !mXDataWidget)
		return;
	JT_XRegister *task = new JT_XRegister(mAccount->client()->rootTask());
	connect(task, SIGNAL(finished()), this, SLOT(slotSentForm()));

	if(mXDataWidget)
	{
		XMPP::XData form;
		form.setFields(mXDataWidget->fields());
		task->setXForm(mForm, form);
	}
	else
	{
		task->setForm(mTranslator->resultData());
	}
	task->go(true);

	//btnRegister->setEnabled(false);
	//btnCancel->setEnabled(false);
}

void dlgRegister::slotSentForm()
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
		//btnRegister->setEnabled(true);
	}
}

#include "dlgregister.moc"
