/*
    msneditaccountwidget.cpp - MSN Account Widget

    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

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

#include "msneditaccountwidget.h"

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

#include "kopeteuiglobal.h"
#include "kopeteglobal.h"

#include "kopetepasswordwidget.h"

#include "msnaccount.h"
#include "msncontact.h"
#include "msneditaccountui.h"
#include "msnnotifysocket.h"

class MSNEditAccountWidgetPrivate
{
public:
	MSNProtocol *protocol;
	KAutoConfig *autoConfig;
	MSNEditAccountUI *ui;
};

MSNEditAccountWidget::MSNEditAccountWidget( MSNProtocol *proto, Kopete::Account *account, QWidget *parent, const char * /* name */ )
: QWidget( parent ), KopeteEditAccountWidget( account )
{
	d = new MSNEditAccountWidgetPrivate;

	d->protocol=proto;

	( new QVBoxLayout( this, 0, 0 ) )->setAutoAdd( true );

	d->ui = new MSNEditAccountUI( this );

	d->autoConfig = new KAutoConfig( d->ui );
	d->autoConfig->addWidget( d->ui->global_settings_page, "MSN" );
	d->autoConfig->retrieveSettings( true );

	// FIXME: actually, I don't know how to set fonts for qlistboxitem - Olivier
	d->ui->label_font->hide();

	// default fields
	if ( account )
	{
		KConfigGroup * config=account->configGroup();
	
		d->ui->m_login->setText( account->accountId() );
		d->ui->m_password->load( &static_cast<MSNAccount *>(account)->password() );

		//remove me after we can change account ids (Matt)
		d->ui->m_login->setDisabled( true );
		d->ui->m_autologin->setChecked( account->excludeConnect()  );
		if ( ( static_cast<MSNAccount*>(account)->serverName() != "messenger.hotmail.com" ) || ( static_cast<MSNAccount*>(account)->serverPort() != 1863) ) {
			d->ui->optionOverrideServer->setChecked( true );
		}

		MSNContact *myself = static_cast<MSNContact *>( account->myself() );

		d->ui->m_displayName->setText( myself->property( Kopete::Global::Properties::self()->nickName()).value().toString() );
		d->ui->m_phw->setText( config->readEntry("PHW") );
		d->ui->m_phm->setText( config->readEntry("PHM") );
		d->ui->m_phh->setText( config->readEntry("PHH") );

		bool connected = account->isConnected();
		if ( connected )
		{
			d->ui->m_warning_1->hide();
			d->ui->m_warning_2->hide();
		}
		d->ui->m_phones->setEnabled( connected );
		d->ui->m_displayName->setEnabled( connected );
		d->ui->m_allowButton->setEnabled( connected );
		d->ui->m_blockButton->setEnabled( connected );

		MSNAccount *m_account = static_cast<MSNAccount*>( account );
		d->ui->m_serverName->setText( m_account->serverName() );
		d->ui->m_serverPort->setValue( m_account->serverPort() );

		QStringList blockList = config->readListEntry( "blockList" );
		QStringList allowList = config->readListEntry( "allowList" );
		//QStringList reverseList =  config->readListEntry("reverseList" );

		for ( QStringList::Iterator it = blockList.begin(); it != blockList.end(); ++it )
			d->ui->m_BL->insertItem( *it );

		for ( QStringList::Iterator it = allowList.begin(); it != allowList.end(); ++it )
			d->ui->m_AL->insertItem( *it );

		d->ui->m_blp->setChecked( config->readEntry( "BLP" ) == "BL" );

		d->ui->m_displayPicture->setPixmap( locateLocal( "appdata", "msnpicture-" +
			account->accountId().lower().replace( QRegExp("[./~]" ), "-" ) + ".png" ) );

		connect( d->ui->m_allowButton, SIGNAL( pressed() ), this, SLOT( slotAllow() ) );
		connect( d->ui->m_blockButton, SIGNAL( pressed() ), this, SLOT( slotBlock() ) );
		connect( d->ui->m_selectImage, SIGNAL( pressed() ), this, SLOT( slotSelectImage() ) );
		connect( d->ui->m_RLButton, SIGNAL( pressed() ), this, SLOT( slotShowReverseList() ) );
		QObject::connect( d->ui->buttonRegister, SIGNAL(clicked()), this, SLOT(slotOpenRegister()));

		d->ui->m_useDisplayPicture->setChecked( config->readBoolEntry( "exportCustomPicture" ));
	}
	else
	{
		d->ui->tab_contacts->setDisabled( true );
		d->ui->tab_info->setDisabled( true );
	}
}

MSNEditAccountWidget::~MSNEditAccountWidget()
{
	delete d;
}

Kopete::Account * MSNEditAccountWidget::apply()
{
	d->autoConfig->saveSettings();

	if ( !account() )
		setAccount( new MSNAccount( d->protocol, d->ui->m_login->text() ) );
	
	KConfigGroup *config=account()->configGroup();

	account()->setExcludeConnect( d->ui->m_autologin->isChecked() );
	d->ui->m_password->save( &static_cast<MSNAccount *>(account())->password() );

	config->writeEntry( "exportCustomPicture", d->ui->m_useDisplayPicture->isChecked() );
	if (d->ui->optionOverrideServer->isChecked() ) {
		config->writeEntry( "serverName", d->ui->m_serverName->text() );
		config->writeEntry( "serverPort", d->ui->m_serverPort->value()  );
	}
	else {
		config->writeEntry( "serverName", "messenger.hotmail.com" );
		config->writeEntry( "serverPort", "1863" );
	}

	static_cast<MSNAccount *>( account() )->resetPictureObject();

	if ( account()->isConnected() )
	{
		MSNContact *myself = static_cast<MSNContact *>( account()->myself() );
		MSNNotifySocket *notify = static_cast<MSNAccount *>( account() )->notifySocket();
		if ( d->ui->m_displayName->text() != myself->property( Kopete::Global::Properties::self()->nickName()).value().toString() )
			static_cast<MSNAccount *>( account() )->setPublicName( d->ui->m_displayName->text() );

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
	return account();
}

bool MSNEditAccountWidget::validateData()
{
	QString userid = d->ui->m_login->text();
	if ( MSNProtocol::validContactId( userid ) )
		return true;

	KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
		i18n( "<qt>You must enter a valid email address.</qt>" ), i18n( "MSN Plugin" ) );
	return false;
}

void MSNEditAccountWidget::slotAllow()
{
	//TODO: play with multiple selection
	QListBoxItem *item = d->ui->m_BL->selectedItem();
	if ( !item )
		return;

	QString handle = item->text();

	MSNNotifySocket *notify = static_cast<MSNAccount *>( account() )->notifySocket();
	if ( !notify )
		return;
	notify->removeContact( handle, 0, MSNProtocol::BL );

	d->ui->m_BL->takeItem( item );
	d->ui->m_AL->insertItem( item );
}

void MSNEditAccountWidget::slotBlock()
{
	//TODO: play with multiple selection
	QListBoxItem *item = d->ui->m_AL->selectedItem();
	if ( !item )
		return;

	QString handle = item->text();

	MSNNotifySocket *notify = static_cast<MSNAccount *>( account() )->notifySocket();
	if ( !notify )
		return;

	notify->removeContact( handle, 0, MSNProtocol::AL );

	d->ui->m_AL->takeItem( item );
	d->ui->m_BL->insertItem( item );
}

void MSNEditAccountWidget::slotShowReverseList()
{
	QStringList reverseList = account()->configGroup()->readListEntry( "reverseList" );
	KMessageBox::informationList( this, i18n( "Here you can see a list of contacts who added you to their contact list" ), reverseList,
		i18n( "Reverse List - MSN Plugin" ) );
}

void MSNEditAccountWidget::slotSelectImage()
{
	// FIXME: the change will take effect immediately, even if the user presses cancel - Olivier
	// FIXME: since we need the accountId to create the file HERE (and it's the problem) we need the account - Olivier
	if ( !account() )
		return;

	QString path = 0;
	bool remoteFile = false;
	KURL filePath = KFileDialog::getImageOpenURL( QString::null, this, i18n( "MSN Display Picture" ) );
	if( filePath.isEmpty() )
		return;

	if( !filePath.isLocalFile() ) {
		if(!KIO::NetAccess::download( filePath, path, this )) {
			KMessageBox::sorry( this, i18n( "Downloading of display image failed" ), i18n( "MSN Plugin" ) );
			return;
		}
		remoteFile = true;
	}
	else path = filePath.path();

	QString futurName = locateLocal( "appdata", "msnpicture-" + account()->accountId().lower().replace( QRegExp( "[./~]" ), "-" ) + ".png" );

	QImage img( path );
	img = img.smoothScale( 96, 96 );
	if ( !img.isNull() && img.save( futurName, "PNG" ) )
	{
		d->ui->m_displayPicture->setPixmap( futurName );
	}
	else
	{
		KMessageBox::sorry( this, i18n( "<qt>An error occurred when trying to change the display picture.<br>"
			"Make sure that you have selected a correct image file</qt>" ), i18n( "MSN Plugin" ) );
	}
	if( remoteFile ) KIO::NetAccess::removeTempFile( path );
}

void MSNEditAccountWidget::slotOpenRegister()
{
	KRun::runURL( "http://register.passport.net/", "text/html" );
}

#include "msneditaccountwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

