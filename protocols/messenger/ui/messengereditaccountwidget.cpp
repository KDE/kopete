/*
    messengereditaccountwidget.cpp - Windows Live Messenger Account Widget

	Copyright (c) 2007		by Zhang Panyong		<pyzhang@gmail.com>
    Copyright (c) 2003      by Olivier Goffart       <ogoffart@kde.org>
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

#include "messengereditaccountwidget.h"

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
//Added by qt3to4:
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

#include "messengeraccount.h"
#include "messengercontact.h"
#include "ui_msneditaccountui.h"
#include "msnnotifysocket.h"
#include "msnprotocol.h"

// TODO: This was using KAutoConfig before, use KConfigXT instead.
class MessengerEditAccountWidgetPrivate
{
public:
	MessengerProtocol *protocol;
	Ui::MessengerEditAccountUI *ui;

	QString pictureUrl;
	QImage pictureData;
};

MessengerEditAccountWidget::MessengerEditAccountWidget( MessengerProtocol *proto, Kopete::Account *account, QWidget *parent )
: QWidget( parent ), KopeteEditAccountWidget( account )
{
	d = new MessengerEditAccountWidgetPrivate;

	d->protocol = proto;

	d->ui = new Ui::MessengerEditAccountUI();
	d->ui->setupUi( this );

	// FIXME: actually, I don't know how to set fonts for qlistboxitem - Olivier
	d->ui->label_font->hide();

	// default fields
	if ( account )
	{
		KConfigGroup * config=account->configGroup();
	
		d->ui->m_login->setText( account->accountId() );
		d->ui->m_password->load( &static_cast<MessengerAccount *>(account)->password() );

		//remove me after we can change account ids (Matt)
		d->ui->m_login->setDisabled( true );
		d->ui->m_autologin->setChecked( account->excludeConnect()  );
		if ( ( static_cast<MessengerAccount*>(account)->serverName() != MESSENGER_DEFAULT_SERVER ) 
				|| ( static_cast<MessengerAccount*>(account)->serverPort() != MESSENGER_DEFAULT_PORT) ) {
			d->ui->optionOverrideServer->setChecked( true );
		}

		d->ui->optionUseHttpMethod->setChecked( static_cast<MessengerAccount*>(account)->useHttpMethod() );
		
		MessengerContact *myself = static_cast<MessengerContact *>( account->myself() );

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

		MessengerAccount *m_account = static_cast<MessengerAccount*>( account );
		d->ui->m_serverName->setText( m_account->serverName() );
		d->ui->m_serverPort->setValue( m_account->serverPort() );

		QStringList blockList = config->readEntry( "blockList", QStringList() );
		QStringList allowList = config->readEntry( "allowList", QStringList() );
		//QStringList reverseList =  config->readListEntry("reverseList" );

		for ( QStringList::Iterator it = blockList.begin(); it != blockList.end(); ++it )
			d->ui->m_BL->insertItem( *it );

		for ( QStringList::Iterator it = allowList.begin(); it != allowList.end(); ++it )
			d->ui->m_AL->insertItem( *it );

		d->ui->m_blp->setChecked( config->readEntry( "BLP" ) == "BL" );

		d->pictureUrl = KStandardDirs::locateLocal( "appdata", "msnpicture-" +
			account->accountId().toLower().replace( QRegExp("[./~]" ), "-" ) + ".png" );
		d->ui->m_displayPicture->setPixmap( d->pictureUrl );

		d->ui->m_useDisplayPicture->setChecked( config->readEntry( "exportCustomPicture", false ));

		// Global Identity
		d->ui->m_globalIdentity->setChecked( config->readEntry("ExcludeGlobalIdentity", false) );
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

MessengerEditAccountWidget::~MessengerEditAccountWidget()
{
	delete d;
}

Kopete::Account * MessengerEditAccountWidget::apply()
{
	if ( !account() )
		setAccount( new MessengerAccount( d->protocol, d->ui->m_login->text() ) );
	
	KConfigGroup *config=account()->configGroup();

	account()->setExcludeConnect( d->ui->m_autologin->isChecked() );
	d->ui->m_password->save( &static_cast<MessengerAccount *>(account())->password() );

	config->writeEntry( "exportCustomPicture", d->ui->m_useDisplayPicture->isChecked() );
	if (d->ui->optionOverrideServer->isChecked() ) {
		config->writeEntry( "serverName", d->ui->m_serverName->text() );
		config->writeEntry( "serverPort", d->ui->m_serverPort->value()  );
	}
	else {
		config->writeEntry( "serverName", MESSENGER_DEFAULT_SERVER );
		config->writeEntry( "serverPort", MESSENGER_DEFAULT_PORT );
	}

	config->writeEntry( "useHttpMethod", d->ui->optionUseHttpMethod->isChecked() );

	// Global Identity
	config->writeEntry( "ExcludeGlobalIdentity", d->ui->m_globalIdentity->isChecked() );

	// Save the avatar image
	if( d->ui->m_useDisplayPicture->isChecked() && !d->pictureData.isNull() )
	{
		d->pictureUrl = KStandardDirs::locateLocal( "appdata", "msnpicture-" +
				account()->accountId().toLower().replace( QRegExp("[./~]" ), "-" ) + ".png" );
		if ( d->pictureData.save( d->pictureUrl, "PNG" ) )
		{
			static_cast<MessengerAccount *>( account() )->setPictureUrl( d->pictureUrl );
		}
		else
		{
			KMessageBox::sorry( this, i18n( "<qt>An error occurred when trying to change the display picture.<br>"
					"Make sure that you have selected a correct image file</qt>" ), i18n( "Messenger Plugin" ) );
		}
	}

	static_cast<MessengerAccount *>( account() )->resetPictureObject();

	if ( account()->isConnected() )
	{
		MessengerContact *myself = static_cast<MessengerContact *>( account()->myself() );
		MessengerNotifySocket *notify = static_cast<MessengerAccount *>( account() )->notifySocket();
		if ( d->ui->m_displayName->text() != myself->property( Kopete::Global::Properties::self()->nickName()).value().toString() )
			static_cast<MessengerAccount *>( account() )->setPublicName( d->ui->m_displayName->text() );

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

/*check if it's the validate Windows Live ID */
bool MessengerEditAccountWidget::validateData()
{
	QString userid = d->ui->m_login->text();
	if ( MessengerProtocol::validContactId( userid ) )
		return true;

	KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
		i18n( "<qt>You must enter a valid email address.</qt>" ), i18n( "Messenger Plugin" ) );
	return false;
}

void MessengerEditAccountWidget::slotAllow()
{
	//TODO: play with multiple selection
	Q3ListBoxItem *item = d->ui->m_BL->selectedItem();
	if ( !item )
		return;

	QString handle = item->text();

	MessengerNotifySocket *notify = static_cast<MSNAccount *>( account() )->notifySocket();
	if ( !notify )
		return;
	notify->removeContact( handle, MSNProtocol::BL, QString::null, QString::null );

	d->ui->m_BL->takeItem( item );
	d->ui->m_AL->insertItem( item );
}

void MessengerEditAccountWidget::slotBlock()
{
	//TODO: play with multiple selection
	Q3ListBoxItem *item = d->ui->m_AL->selectedItem();
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

/*show reverse list*/
void MessengerEditAccountWidget::slotShowReverseList()
{
	QStringList reverseList = account()->configGroup()->readEntry( "reverseList", QStringList() );
	KMessageBox::informationList( this, i18n( "Here you can see a list of contacts who added you to their contact list" ), reverseList,
		i18n( "Reverse List - Messenger Plugin" ) );
}

void MessengerEditAccountWidget::slotSelectImage()
{
	QString path = 0;
	bool remoteFile = false;
	KUrl filePath = KFileDialog::getImageOpenUrl( KUrl(), this, i18n( "Windows Live Messenger Display Picture" ) );
	if( filePath.isEmpty() )
		return;

	if( !filePath.isLocalFile() ) {
		/*access remote file*/
		if(!KIO::NetAccess::download( filePath, path, this )) {
			KMessageBox::sorry( this, i18n( "Downloading of display image failed" ), i18n( "Messenger Plugin" ) );
			return;
		}
		remoteFile = true;
	}
	else path = filePath.path();

	QImage img( path );
	img = KPixmapRegionSelectorDialog::getSelectedImage( QPixmap::fromImage(img), 96, 96, this );

	if(!img.isNull()) 
	{
		img = MSNProtocol::protocol()->scalePicture(img);
	
		d->ui->m_displayPicture->setPixmap( QPixmap::fromImage(img) );
		d->pictureData = img;
	}
	else
	{
		KMessageBox::sorry( this, i18n( "<qt>An error occurred when trying to change the display picture.<br>"
			"Make sure that you have selected a correct image file</qt>" ), i18n( "Messenger Plugin" ) );
	}
	if( remoteFile ) KIO::NetAccess::removeTempFile( path );
}

void MessengerEditAccountWidget::slotOpenRegister()
{
	/*register windows Live ID*/
	KToolInvocation::invokeBrowser( "https://accountservices.passport.net/reg.srf" );
}

#include "messengereditaccountwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

