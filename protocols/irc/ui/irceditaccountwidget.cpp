/*
    msneditaccountwidget.cpp - MSN Identity Widget

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
#include "ircprotocol.h"
#include "irceditaccountwidget.h"

IRCEditIdentityWidget::IRCEditIdentityWidget(const IRCProtocol *proto, IRCIdentity *ident, QWidget *parent, const char * )
				  : IRCEditIdentityBase(parent), EditIdentityWidget(ident)
{
	mProtocol = proto;

	if( m_identity )
	{
		QString nickName = m_identity->identityId().section( '@', 0, 0);
		QString serverInfo = m_identity->identityId().section( '@', 1);

		mNickName->setText( nickName );
		mServer->setText( serverInfo.section(':', 0, 0) );
		mPort->setValue( serverInfo.section(':',1).toUInt() );

		if(m_identity->rememberPassword()) mPassword->setText( m_identity->getPassword() );
	}
}

IRCEditIdentityWidget::~IRCEditIdentityWidget()
{
}

KopeteIdentity *IRCEditIdentityWidget::apply()
{
	QString mIdentityId = mNickName->text() + QString::fromLatin1("@") + mServer->text() + QString::fromLatin1(":") + QString::number( mPort->value() );

	if( !m_identity )
		m_identity = new IRCIdentity( mIdentityId, mProtocol );
	else
		m_identity->setIdentityId( mIdentityId );

	m_identity->setPassword( mPassword->text() );
	m_identity->setAutoLogin( mAutoConnect->isChecked() );

	return m_identity;
}


bool IRCEditIdentityWidget::validateData()
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
