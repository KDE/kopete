/*
    qqeditaccountwidget.cpp - QQ Account Widget

    Copyright (c) 2003      by Olivier Goffart <ogoffart@kde.org>
    Copyright (c) 2003      by Martijn Klingens <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "qqeditaccountwidget.h"

#include <qcheckbox.h>
#include <q3groupbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <q3listbox.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <QPixmap>
#include <QVBoxLayout>
#include <QLatin1String>

#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <ktoolinvocation.h>
#include <kconfig.h>
#include <kpixmapregionselectordialog.h>
#include <kconfiggroup.h>

#include "kopeteuiglobal.h"
#include "kopeteglobal.h"

#include "kopetepasswordwidget.h"

#include "qqaccount.h"
#include "qqcontact.h"
#include "ui_qqeditaccountui.h"
#include "qqnotifysocket.h"
#include "qqprotocol.h"
#include "dlgqqvcard.h"

// TODO: This was using KAutoConfig before, use KConfigXT instead.
class QQEditAccountWidgetPrivate
{
public:
	QQProtocol *protocol;
	Ui::QQEditAccountUI *ui;

	QString pictureUrl;
	QImage pictureData;
};

QQEditAccountWidget::QQEditAccountWidget( QQProtocol *proto, Kopete::Account *account, QWidget *parent )
: QWidget( parent ), KopeteEditAccountWidget( account )
{
	d = new QQEditAccountWidgetPrivate;

	d->protocol=proto;

	d->ui = new Ui::QQEditAccountUI();
	d->ui->setupUi( this );

	// default fields
	if ( account )
	{
		// Basic Setup
		d->ui->m_login->setText( account->accountId() );
		d->ui->m_password->load( &static_cast<QQAccount *>(account)->password() );
		//remove me after we can change account ids (Matt)
		d->ui->m_login->setReadOnly( true );
		d->ui->m_autologin->setChecked( account->excludeConnect()  );

		QQContact *myself = static_cast<QQContact *>( account->myself() );
		if( myself )
			connect( d->ui->buttonVCard, SIGNAL(clicked()), myself, SLOT(slotUserInfo()));
			
		QQAccount *m_account = static_cast<QQAccount*>( account );
		d->ui->m_serverName->setText( m_account->serverName() );
		d->ui->m_serverPort->setValue( m_account->serverPort() );
		if ( ( m_account->serverName() != "tcpconn.tencent.com" ) || ( m_account->serverPort() != 80) ) {
			d->ui->optionOverrideServer->setChecked( true );
			d->ui->m_serverName->setEnabled(true);
			d->ui->m_serverPort->setEnabled(true);
		}
	}

	connect( d->ui->buttonRegister, SIGNAL(clicked()), this, SLOT(slotOpenRegister()));
	QWidget::setTabOrder( d->ui->m_login, d->ui->m_password->mRemembered );
	QWidget::setTabOrder( d->ui->m_password->mRemembered, d->ui->m_password->mPassword );
	QWidget::setTabOrder( d->ui->m_password->mPassword, d->ui->m_autologin );
}

QQEditAccountWidget::~QQEditAccountWidget()
{
	delete d->ui;
	delete d;
}

Kopete::Account * QQEditAccountWidget::apply()
{
	if ( !account() )
		setAccount( new QQAccount( d->protocol, d->ui->m_login->text() ) );
	
	KConfigGroup *config=account()->configGroup();

	account()->setExcludeConnect( d->ui->m_autologin->isChecked() );
	d->ui->m_password->save( &static_cast<QQAccount *>(account())->password() );

	if (d->ui->optionOverrideServer->isChecked() ) {
		config->writeEntry( "serverName", d->ui->m_serverName->text().trimmed() );
		config->writeEntry( "serverPort", d->ui->m_serverPort->value()  );
	}
	else {
		config->writeEntry( "serverName", "tcpconn.tencent.com" );
		config->writeEntry( "serverPort", "80" );
	}

		/*
	if ( account()->isConnected() )
	{
		QQContact *myself = static_cast<QQContact *>( account()->myself() );
		QQNotifySocket *notify = static_cast<QQAccount *>( account() )->notifySocket();
		if ( d->ui->m_nickName->text() != myself->property( Kopete::Global::Properties::self()->nickName()).value().toString() )
		;
			// static_cast<QQAccount *>( account() )->setPublicName( d->ui->m_displayName->text() );

		if ( notify )
		{
			if ( d->ui->m_phw->text() != myself->phoneWork() && ( !d->ui->m_phw->text().isEmpty() || !myself->phoneWork().isEmpty() ) )
				notify->changePhoneNumber( "PHW", d->ui->m_phw->text() );
			if( d->ui->m_phh->text() != myself->phoneHome() && ( !d->ui->m_phh->text().isEmpty() || !myself->phoneHome().isEmpty() ) )
				notify->changePhoneNumber( "PHH", d->ui->m_phh->text() );
			if( d->ui->m_phm->text() != myself->phoneMobile() && ( !d->ui->m_phm->text().isEmpty() || !myself->phoneMobile().isEmpty() ) )
				notify->changePhoneNumber( "PHM", d->ui->m_phm->text() );
			// (the && .isEmpty is because one can be null and the other empty)

			if ( ( config->readEntry("BLP") == "BL" ) != d->ui->m_blp->isChecked() )
			{

				// Yes, I know, calling sendCommand here is not very clean - Olivier
				notify->sendCommand( "BLP", d->ui->m_blp->isChecked() ? "BL" : "AL" );
			}
		}
	}
		*/
	return account();
}

bool QQEditAccountWidget::validateData()
{
	QString userid = d->ui->m_login->text();
	if ( QQProtocol::validContactId( userid ) )
		return true;

	KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
		i18n( "<qt>You must enter a valid email address.</qt>" ), i18n( "QQ Plugin" ) );
	return false;
}

void QQEditAccountWidget::slotOpenRegister()
{
	KToolInvocation::invokeBrowser( "http://freereg.qq.com/"  );
}

#include "qqeditaccountwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

