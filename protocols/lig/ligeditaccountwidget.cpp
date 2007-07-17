/*
    ligeditaccountwidget.h - Kopete Lig Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "ligeditaccountwidget.h"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <kcombobox.h>

#include <kautoconfig.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <kpassdlg.h>
#include <krun.h>
#include <kconfig.h>
#include <kpixmapregionselectordialog.h>

#include "kopeteuiglobal.h"
#include "kopeteglobal.h"

#include "kopetepasswordwidget.h"
#include "kopeteaccountmanager.h"

#include "ligaccountpreferences.h"
#include "ligaccount.h"
#include "ligprotocol.h"

class LigEditAccountWidgetPrivate
{
public:
	LigProtocol *protocol;
	KAutoConfig *autoConfig;
	LigAccountPreferences *ui;

//	QString pictureUrl;
//	QImage pictureData;
};

LigEditAccountWidget::LigEditAccountWidget( QWidget* parent, Kopete::Account* account): QWidget( parent ), KopeteEditAccountWidget( account )
{
	d = new LigEditAccountWidgetPrivate;

	( new QVBoxLayout( this ) )->setAutoAdd( true );
				kdDebug(14210) << k_funcinfo << endl;
//	m_preferencesWidget = new LigAccountPreferences( this );
	d->ui = new LigAccountPreferences( this );

	if ( account )
	{
		KConfigGroup * config=account->configGroup();
	
		d->ui->m_login->setText( account->accountId() );
		d->ui->m_password->load( &static_cast<LigAccount *>(account)->password() );

		//remove me after we can change account ids (Matt)
		d->ui->m_login->setDisabled( true );
		d->ui->m_autologin->setChecked( account->excludeConnect()  );
		if ( ( static_cast<LigAccount*>(account)->sipServerName() != "sip.melig.com.br" ) || ( static_cast<LigAccount*>(account)->sipServerPort() != 5060)
			|| ( static_cast<LigAccount*>(account)->stunServerName() != "stun.melig.com.br" ) || ( static_cast<LigAccount*>(account)->stunServerPort() != 3478) ) {
			d->ui->optionOverrideServer->setChecked( true );
		}
	}

	connect( d->ui->buttonRegister, SIGNAL(clicked()), this, SLOT(slotOpenRegister()));
	QWidget::setTabOrder( d->ui->m_login, d->ui->m_password->mRemembered );
	QWidget::setTabOrder( d->ui->m_password->mRemembered, d->ui->m_password->mPassword );
	QWidget::setTabOrder( d->ui->m_password->mPassword, d->ui->m_autologin );

}

LigEditAccountWidget::~LigEditAccountWidget()
{
}

Kopete::Account* LigEditAccountWidget::apply()
{
	QString accountName;
	if ( d->ui->m_login->text().isEmpty() )
		accountName = "Lig Account";
	else
		accountName = d->ui->m_login->text();
	
	if ( account() )
		// FIXME: ? account()->setAccountLabel(accountName);
		account()->myself()->setProperty( Kopete::Global::Properties::self()->nickName(), accountName );
	else
		setAccount( new LigAccount( LigProtocol::protocol(), accountName ) );

	return account();
}

bool LigEditAccountWidget::validateData()
{
    //return !( m_preferencesWidget->m_acctName->text().isEmpty() );
	return true;
}

void LigEditAccountWidget::slotOpenRegister()
{
	KRun::runURL( "https://cadastro.ig.com.br/cadastro/origem.do?origem=LIG", "text/html" );
}

#include "ligeditaccountwidget.moc"
