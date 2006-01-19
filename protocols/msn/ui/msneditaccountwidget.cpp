/*
    msneditaccountwidget.cpp - MSN Account Widget

    Copyright (c) 2003      by Olivier Goffart       <ogoffart @ kde.org>
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

#include "msnaccount.h"
#include "msncontact.h"
#include "msneditaccountui.h"
#include "msnnotifysocket.h"
#include "msnprotocol.h"

class MSNEditAccountWidgetPrivate
{
public:
	MSNProtocol *protocol;
	KAutoConfig *autoConfig;
	MSNEditAccountUI *ui;

	QString pictureUrl;
	QImage pictureData;
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
	d->autoConfig->addWidget( d->ui->privacy_page, "MSN" );
	//the JabberAccount need to be saved as text, and can't be handled by kautoconfig
	d->autoConfig->ignoreSubWidget( d->ui->JabberAccount );
	d->autoConfig->retrieveSettings( true );
	
	//Get a list of all jabber accounts
	KGlobal::config()->setGroup("MSN");
	QString jab_account=KGlobal::config()->readEntry("JabberAccount");
	
	QPtrList<Kopete::Account>  accounts = Kopete::AccountManager::self()->accounts();
	for(Kopete::Account *a=accounts.first() ; a; a=accounts.next() )
	{
		if(a->protocol()->pluginId()=="JabberProtocol")
		{
			d->ui->JabberAccount->insertItem(a->accountId());
			if( jab_account.isEmpty() )
				jab_account=a->accountId();
		}
	}
	d->ui->JabberAccount->setCurrentText(jab_account);

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
		
		d->ui->m_webcamPort->setDisabled(true);
		uint port=config->readNumEntry("WebcamPort" ,0);
		d->ui->m_useWebcamPort->setChecked( port != 0);
		d->ui->m_webcamPort->setValue( port != 0 ? port : 6891 );

		d->ui->optionUseHttpMethod->setChecked( static_cast<MSNAccount*>(account)->useHttpMethod() );
		
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

		d->pictureUrl = locateLocal( "appdata", "msnpicture-" +
			account->accountId().lower().replace( QRegExp("[./~]" ), "-" ) + ".png" );
		d->ui->m_displayPicture->setPixmap( d->pictureUrl );

		d->ui->m_useDisplayPicture->setChecked( config->readBoolEntry( "exportCustomPicture" ));

		// Global Identity
		d->ui->m_globalIdentity->setChecked( config->readBoolEntry("ExcludeGlobalIdentity", false) );
	}
	else
	{
		d->ui->tab_contacts->setDisabled( true );
		d->ui->m_displayName->setDisabled( true );
		d->ui->m_phones->setDisabled( true );
	}

	connect( d->ui->m_allowButton, SIGNAL( clicked() ), this, SLOT( slotAllow() ) );
	connect( d->ui->m_blockButton, SIGNAL( clicked() ), this, SLOT( slotBlock() ) );
	connect( d->ui->m_selectImage, SIGNAL( clicked() ), this, SLOT( slotSelectImage() ) );
	connect( d->ui->m_RLButton, SIGNAL( clicked() ), this, SLOT( slotShowReverseList() ) );
	connect( d->ui->buttonRegister, SIGNAL(clicked()), this, SLOT(slotOpenRegister()));
	QWidget::setTabOrder( d->ui->m_login, d->ui->m_password->mRemembered );
	QWidget::setTabOrder( d->ui->m_password->mRemembered, d->ui->m_password->mPassword );
	QWidget::setTabOrder( d->ui->m_password->mPassword, d->ui->m_autologin );
}

MSNEditAccountWidget::~MSNEditAccountWidget()
{
	delete d;
}

Kopete::Account * MSNEditAccountWidget::apply()
{
	d->autoConfig->saveSettings();
	KGlobal::config()->setGroup("MSN");
	KGlobal::config()->writeEntry("JabberAccount", d->ui->JabberAccount->currentText());

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

	config->writeEntry( "useHttpMethod", d->ui->optionUseHttpMethod->isChecked() );
	
	if(d->ui->m_useWebcamPort->isChecked())
		config->writeEntry( "WebcamPort" , d->ui->m_webcamPort->value() );
	else
		config->writeEntry( "WebcamPort" , 0 );

	// Global Identity
	config->writeEntry( "ExcludeGlobalIdentity", d->ui->m_globalIdentity->isChecked() );

	// Save the avatar image
	if( d->ui->m_useDisplayPicture->isChecked() && !d->pictureData.isNull() )
	{
		d->pictureUrl = locateLocal( "appdata", "msnpicture-" +
				account()->accountId().lower().replace( QRegExp("[./~]" ), "-" ) + ".png" );
		if ( d->pictureData.save( d->pictureUrl, "PNG" ) )
		{
			static_cast<MSNAccount *>( account() )->setPictureUrl( d->pictureUrl );
		}
		else
		{
			KMessageBox::sorry( this, i18n( "<qt>An error occurred when trying to change the display picture.<br>"
					"Make sure that you have selected a correct image file</qt>" ), i18n( "MSN Plugin" ) );
		}
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
	notify->removeContact( handle, MSNProtocol::BL, QString::null, QString::null );

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

	notify->removeContact( handle, MSNProtocol::AL, QString::null, QString::null );

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

	QImage img( path );
	img = KPixmapRegionSelectorDialog::getSelectedImage( QPixmap(img), 96, 96, this );

	if(!img.isNull()) 
	{
		img = MSNProtocol::protocol()->scalePicture(img);
	
		d->ui->m_displayPicture->setPixmap( QPixmap(img) );
		d->pictureData = img;
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

