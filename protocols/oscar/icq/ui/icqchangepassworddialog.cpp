/*
   Kopete Oscar Protocol
   icqchangepassworddialog.cpp - ICQ change password dialog

   Copyright (c) 2006 Roman Jarosz <kedgedev@centrum.cz>

   Kopete (c) 2006 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "icqchangepassworddialog.h"
#include "ui_icqchangepassword.h"

#include <kmessagebox.h>

#include "icqaccount.h"

ICQChangePasswordDialog::ICQChangePasswordDialog( ICQAccount *account, QWidget *parent )
	: KDialog( parent ), m_account( account )
{
	setCaption( i18n( "Change ICQ Password" ) );
	setButtons( KDialog::Ok | KDialog::Cancel );

	m_ui = new Ui::ICQChangePassword();
	QWidget *w = new QWidget( this );
	m_ui->setupUi( w );
	setMainWidget( w );

	connect( m_account->engine(), SIGNAL(icqPasswordChanged(bool)),
	         this, SLOT(slotPasswordChanged(bool)) );
}

ICQChangePasswordDialog::~ICQChangePasswordDialog()
{
	delete m_ui;
}

void ICQChangePasswordDialog::slotButtonClicked( int button )
{
	if ( button == KDialog::Ok )
	{
		if ( !m_account->engine()->isActive() )
		{
			if ( KMessageBox::questionYesNo( this,
			                                 i18n ( "Your account needs to be connected before the password can be changed. Do you want to try to connect now?" ),
			                                 i18n ( "ICQ Password Change" ), KGuiItem( i18n("Connect") ), KGuiItem( i18n("Stay Offline") ) ) == KMessageBox::Yes )
			{
				m_account->connect();
			}
			return;
		}

		if ( m_ui->currentPassword->text().isEmpty()
		     || ( m_account->engine()->password() != m_ui->currentPassword->text() ) )
		{
			KMessageBox::queuedMessageBox( this, KMessageBox::Sorry,
			                               i18n( "You entered your current password incorrectly." ),
			                               i18n( "Password Incorrect" ) );
			return;
		}

		if ( m_ui->newPassword1->text() != m_ui->newPassword2->text() )
		{
			KMessageBox::queuedMessageBox( this, KMessageBox::Sorry,
			                               i18n( "Your new passwords do not match. Please enter them again." ),
			                               i18n( "Password Incorrect" ) );
			return;
		}

		if ( m_ui->newPassword1->text().length() < 6 || 8 < m_ui->newPassword1->text().length() )
		{
			KMessageBox::queuedMessageBox( this, KMessageBox::Sorry,
			                               i18n( "Your new password must be between 6-8 characters long." ),
			                               i18n( "Password Incorrect" ) );
			return;
		}

		if ( !m_account->engine()->changeICQPassword( m_ui->newPassword1->text() ) )
			KMessageBox::queuedMessageBox ( dynamic_cast<QWidget*>(parent()), KMessageBox::Sorry,
			                                i18n ( "Your password could not be changed." ) );
	}
	else if ( button == KDialog::Cancel )
	{
		reject();
	}
}

void ICQChangePasswordDialog::slotPasswordChanged( bool error )
{
	if ( !error )
	{
		KMessageBox::queuedMessageBox( dynamic_cast<QWidget*>(parent()), KMessageBox::Information,
		                               i18n( "Your password has been changed successfully." ) );
	}
	else
	{
		KMessageBox::queuedMessageBox ( dynamic_cast<QWidget*>(parent()), KMessageBox::Sorry,
		                                i18n ( "Your password could not be changed." ) );
	}
	accept();
}

#include "icqchangepassworddialog.moc"
