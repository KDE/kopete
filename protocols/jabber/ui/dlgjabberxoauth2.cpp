/*
    dlgjabberxoauth2.cpp - X-OAuth2 dialog

    Copyright (c) 2016 by Pali Rohár <pali.rohar@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "dlgjabberxoauth2.h"

#include <klocale.h>
#include <kopetepassword.h>

#include "jabberaccount.h"
#include "ui_dlgxoauth2.h"

DlgJabberXOAuth2::DlgJabberXOAuth2(JabberAccount *account, QWidget *parent) : KDialog(parent), m_account(account) {

	setCaption(i18n("Manage X-OAuth2 tokens"));
	setButtons(KDialog::Ok | KDialog::Cancel);
	setDefaultButton(KDialog::Ok);
	showButtonSeparator(true);

	m_mainWidget = new Ui::DlgXOAuth2();
	m_mainWidget->setupUi(mainWidget());

	const QString &token = m_account->password().cachedValue();
	if (token.contains(QChar(0x7F))) {
		const QStringList &tokens = token.split(QChar(0x7F));
		if (tokens.size() == 5) {
			m_mainWidget->cbUseAccessToken->setChecked(!tokens.at(3).isEmpty());
			m_mainWidget->clientId->setText(tokens.at(0));
			m_mainWidget->clientSecretKey->setText(tokens.at(1));
			m_mainWidget->refreshToken->setText(tokens.at(2));
			m_mainWidget->accessToken->setText(tokens.at(3));
			m_mainWidget->requestUrl->setText(tokens.at(4));
		}
	}

	connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
	connect(this, SIGNAL(cancelClicked()), this, SLOT(slotCancel()));

}

DlgJabberXOAuth2::~DlgJabberXOAuth2() {

	delete m_mainWidget;

}

void DlgJabberXOAuth2::slotOk() {

	QStringList tokens;
	tokens << m_mainWidget->clientId->text();
	tokens << m_mainWidget->clientSecretKey->text();
	tokens << m_mainWidget->refreshToken->text();
	tokens << m_mainWidget->accessToken->text();
	tokens << m_mainWidget->requestUrl->text();
	m_account->password().set(tokens.join(QChar(0x7F)));

}

void DlgJabberXOAuth2::slotCancel() {

	deleteLater();

}

#include "dlgjabberxoauth2.moc"
