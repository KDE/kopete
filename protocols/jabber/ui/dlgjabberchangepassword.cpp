
/***************************************************************************
                   Change the password of a Jabber account
                             -------------------
    begin                : Tue May 31 2005
    copyright            : (C) 2005 by Till Gerken <till@tantalo.net>

		Kopete (C) 2001-2005 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dlgjabberchangepassword.h"

#include <kdebug.h>
#include <klocale.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <kopetepassword.h>
#include <xmpp_tasks.h>
#include "jabberaccount.h"
#include "ui_dlgchangepassword.h"

DlgJabberChangePassword::DlgJabberChangePassword ( JabberAccount *account, QWidget *parent )
 : KDialog ( parent )
{
	setCaption( i18n("Change Jabber Password") );
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );
	showButtonSeparator( true );

	m_account = account;

	new QWidget( this );
	m_mainWidget = new Ui::DlgChangePassword;
	m_mainWidget->setupUi( mainWidget() );
    
    m_mainWidget->peNewPassword1->setPasswordMode( true );
    m_mainWidget->peNewPassword2->setPasswordMode( true );
    m_mainWidget->peCurrentPassword->setPasswordMode( true );
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(cancelClicked()),this,SLOT(slotCancel()));
}

DlgJabberChangePassword::~DlgJabberChangePassword()
{
	delete m_mainWidget;
}

void DlgJabberChangePassword::slotOk ()
{
	if ( m_account->password().cachedValue () != m_mainWidget->peCurrentPassword->text() )
	{
		KMessageBox::queuedMessageBox ( this, KMessageBox::Sorry,
							 i18n ( "You entered your current password incorrectly." ),
							 i18n ( "Password Incorrect" ) );
		return;
	}

	if ( m_mainWidget->peNewPassword1->text() !=  m_mainWidget->peNewPassword2->text() )
	{
		KMessageBox::queuedMessageBox ( this, KMessageBox::Sorry,
							 i18n ( "Your new passwords do not match. Please enter them again." ),
							 i18n ( "Password Incorrect" ) );
		return;
	}

	if ( m_mainWidget->peNewPassword1->text().isEmpty() ) 
	{
		KMessageBox::queuedMessageBox ( this, KMessageBox::Sorry,
							 i18n ( "For security reasons, you are not allowed to set an empty password." ),
							 i18n ( "Password Incorrect" ) );
		return;
	}

	if ( !m_account->isConnected () )
	{
		if ( KMessageBox::questionYesNo ( this,
										  i18n ( "Your account needs to be connected before the password can be changed. Do you want to try to connect now?" ),
		                                  i18n ( "Jabber Password Change" ), KGuiItem( i18n("Connect") ), KGuiItem( i18n("Stay Offline") ) ) == KMessageBox::Yes )
		{
			connect ( m_account, SIGNAL (isConnectedChanged()), this, SLOT (slotChangePassword()) );
			m_account->connect ();
		}
	}
	else
	{
		slotChangePassword();
	}

}

void DlgJabberChangePassword::slotCancel ()
{

	deleteLater ();

}

void DlgJabberChangePassword::slotChangePassword ()
{

	XMPP::JT_Register *task = new XMPP::JT_Register ( m_account->client()->rootTask () );
	QObject::connect ( task, SIGNAL (finished()), this, SLOT (slotChangePasswordDone()) );

	task->changepw ( m_mainWidget->peNewPassword1->text () );
	task->go ( true );

}

void DlgJabberChangePassword::slotChangePasswordDone ()
{

	XMPP::JT_Register *task = (XMPP::JT_Register *) sender ();

	if ( task->success () )
	{
		KMessageBox::queuedMessageBox ( dynamic_cast<QWidget*>(parent()), KMessageBox::Information,
								   i18n ( "Your password has been changed successfully. Please note that the change may not be instantaneous. If you have problems logging in with your new password, please contact the administrator." ),
								   i18n ( "Jabber Password Change" ) );

		m_account->password().set ( m_mainWidget->peNewPassword1->text () );
	}
	else
	{
		KMessageBox::queuedMessageBox ( dynamic_cast<QWidget*>(parent()), KMessageBox::Sorry, 
							 i18n ( "Your password could not be changed. Either your server does not support this feature or the administrator does not allow you to change your password." ) );
	}

	deleteLater();

}

#include "dlgjabberchangepassword.moc"
