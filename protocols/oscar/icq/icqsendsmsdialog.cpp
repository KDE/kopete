/*
    icqreadaway.cpp  -  ICQ Protocol Plugin

    Copyright (c) 2003 by Stefan Gehn <metz AT gehn.net>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "icqsendsmsdialog.h"
#include "icqprotocol.h"
#include "icqaccount.h"
#include "icqcontact.h"

#include <qhbox.h>
#include <qvbox.h>

#include <qlabel.h>
#include <klineedit.h>
#include <ktextedit.h>
#include <klocale.h>
#include <krun.h>

#include <assert.h>


ICQSendSMSDialog::ICQSendSMSDialog(ICQAccount *acc, ICQContact *c, QWidget *parent, const char* name)
	: KDialogBase(parent, name, false, QString::null, Close | User1,
		Close, false, i18n("&Send SMS"))
{
	assert(acc);

	mContact = c;
	mAccount = acc;

	if (c)
		setCaption(i18n("Send SMS to %1").arg(c->displayName()));
	else
		setCaption(i18n("Send SMS"));

	QVBox *mMainWidget = makeVBoxMainWidget();
	QHBox *numberBox = new QHBox(mMainWidget);
	lblNumber = new QLabel(i18n("Number:"), numberBox, "lblNumber");
	edtNumber = new KLineEdit(numberBox, "edtNumber");

	/*if(c)
	{
		edtNumber->setText(c->mobileNumber());
	}*/

	lblMessage = new QLabel(i18n("Message:"), mMainWidget, "lblMessageNumber");
	edtMessage = new KTextEdit(mMainWidget, "edtMessage");

	connect(this, SIGNAL(user1Clicked()),
		this, SLOT(slotSendShortMessage()));
	connect(this, SIGNAL(closeClicked()),
		this, SLOT(slotCloseClicked()));
}

void ICQSendSMSDialog::slotSendShortMessage()
{
	if(!mAccount->isConnected() || edtMessage->text().isEmpty() || edtNumber->text().isEmpty())
		return;

	//mMainWidget->setDisabled(true);
	//enableButton(User1, false);

	QString num = edtNumber->text();
	QString msg = edtMessage->text();
	QString uin = mAccount->myself()->contactId();
	QString nick = mAccount->myself()->displayName();
	mAccount->engine()->sendCLI_SENDSMS( num, msg, uin, nick );

	accept();
} // END slotSendShortMessage()

void ICQSendSMSDialog::slotCloseClicked()
{
	emit closing();
}

#include "icqsendsmsdialog.moc"
// vim: set noet ts=4 sts=4 sw=4:
