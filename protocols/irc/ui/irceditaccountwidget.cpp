/*
    msneditaccountwidget.cpp - MSN Account Widget

    Copyright (c) 2003 by Olivier Goffart  <ogoffart@tiscalinet.be>

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
#include <kmessagebox.h>
#include <klocale.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>

#include "ircaccount.h"
#include "irceditaccountwidget.h"

IRCEditAccountWidget::IRCEditAccountWidget(IRCProtocol *proto, IRCAccount *ident, QWidget *parent, const char * )
				  : IRCEditAccountBase(parent), EditAccountWidget(ident)
{
	mProtocol = proto;

	if( m_account )
	{
		QString nickName = m_account->accountId().section( '@', 0, 0);
		QString serverInfo = m_account->accountId().section( '@', 1);

		mNickName->setText( nickName );
		mServer->setText( serverInfo.section(':', 0, 0) );
		mPort->setValue( serverInfo.section(':',1).toUInt() );

		mNickName->setReadOnly(true);
		mServer->setReadOnly(true);


		if(m_account->rememberPassword()) mPassword->setText( m_account->getPassword() );
	}
}

IRCEditAccountWidget::~IRCEditAccountWidget()
{
}

KopeteAccount *IRCEditAccountWidget::apply()
{
	QString mAccountId = mNickName->text() + QString::fromLatin1("@") + mServer->text() + QString::fromLatin1(":") + QString::number( mPort->value() );

	if( !m_account )
		m_account = new IRCAccount( mProtocol, mAccountId );
//	else
//		m_account->setAccountId( mAccountId );

	m_account->setPassword( mPassword->text() );
	m_account->setAutoLogin( mAutoConnect->isChecked() );

	return m_account;
}


bool IRCEditAccountWidget::validateData()
{
	if( mNickName->text().isEmpty() )
		KMessageBox::sorry(this, i18n("<qt>You must enter a nickname</qt>"), i18n("Kopete"));
	else if( mServer->text().isEmpty() )
		KMessageBox::sorry(this, i18n("<qt>You must enter a server</qt>"), i18n("Kopete"));
	else
		return true;

	return false;
}

#include "irceditaccountwidget.moc"
