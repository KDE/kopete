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

#include <qlayout.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "msneditaccountwidget.h"
#include "msnprotocol.h"
#include "msnaccount.h"

MSNEditIdentityWidget::MSNEditIdentityWidget(MSNProtocol *proto, KopeteIdentity *ident, QWidget *parent, const char * )
				  : QWidget(parent), EditIdentityWidget(ident)
{
	//default fields
	QVBoxLayout *layout=new QVBoxLayout(this);
	layout->setAutoAdd(true);

	new QLabel( i18n ("Login") , this );
	m_login = new QLineEdit( this );

	new QLabel( i18n("Password"), this);
	m_password = new QLineEdit( this );
	m_password->setEchoMode( QLineEdit::Password );

	m_rememberpasswd = new QCheckBox( i18n("Remember Password") , this );
	m_autologin = new QCheckBox( i18n("Auto Login") , this );
	

	layout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding ));

	if(ident)
	{
		if(ident->rememberPassword())
		{
			m_rememberpasswd->setChecked(true);
			m_password->setText(ident->getPassword());
		}
		m_login->setText(ident->identityId());
		m_autologin->setChecked((ident && ident->autoLogin()));
	}
	else
		m_rememberpasswd->setChecked(true);

	m_protocol=proto;
}

MSNEditIdentityWidget::~MSNEditIdentityWidget()
{
}

KopeteIdentity *MSNEditIdentityWidget::apply()
{
	if(!m_identity)
		m_identity=new MSNIdentity(m_protocol, m_login->text() );
	if(m_rememberpasswd->isChecked())
	{
		m_identity->setPassword( m_password->text() );
	}
	else
		m_identity->setPassword( QString::null );

	m_identity->setAutoLogin(m_autologin->isChecked());
	return m_identity;
}


bool MSNEditIdentityWidget::validateData()
{
	QString userid = m_login->text();
	if( MSNProtocol::validContactId(userid) )
		return true;

	KMessageBox::sorry(this, i18n("<qt>You must enter a valid e-mail address as login</qt>"), i18n("MSN Messenger"));
	return false;
}

#include "msneditaccountwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

