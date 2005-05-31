/*
    kopeteidentityconfig.cpp  -  Kopete Identity config page

    Copyright (c) 2005      by Michaël Larouche       <shock@shockdev.ca.tc>

    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteidentityconfig.h"

#include <qlayout.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <kfiledialog.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kglobal.h>

#include "kopeteglobal.h"
#include "kopeteidentityconfigbase.h"
#include "kopeteaccountmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetecontactlist.h"

#include "kopeteconfig.h"

typedef KGenericFactory<KopeteIdentityConfig, QWidget> KopeteIdentityConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_identityconfig, KopeteIdentityConfigFactory( "kcm_kopete_identityconfig" ) )

KopeteIdentityConfig::KopeteIdentityConfig(QWidget *parent, const char *name, const QStringList &args) : KCModule( KopeteIdentityConfigFactory::instance(), parent, args)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	m_view = new KopeteIdentityConfigBase( this, "KopeteIdentityConfig::m_view" );

	addConfig( Kopete::Config::self(), m_view );
	// Populate the Account Combo Box
	QPtrList<Kopete::Account>  accounts = Kopete::AccountManager::self()->accounts();
	for(Kopete::Account *i=accounts.first() ; i; i=accounts.next() )
	{
		QString accountName = i->accountLabel();
		QPixmap accountIcon = i->accountIcon();
		m_view->kcfg_GlobalIdentityAccountSelected->insertItem(accountIcon, accountName);
		// Populate QMap for futher use
		m_listAccounts.insert(accountName, i);
	}

	load();

	// Action signal/slots
	connect(m_view->m_selectImage, SIGNAL(clicked()), this, SLOT(slotSelectImage()));

	// Settings signal/slots
	connect(this, SIGNAL(changed()), this, SLOT(slotSettingsChanged()));
}

void KopeteIdentityConfig::load()
{
	KCModule::load();
}

void KopeteIdentityConfig::save()
{
	KCModule::save();
	load();

	// Apply the global identity
	Kopete::ContactList::self()->loadGlobalIdentity();
}

void KopeteIdentityConfig::slotSelectImage()
{
	QString path = 0;
	bool remoteFile = false;
	KURL filePath = KFileDialog::getImageOpenURL( QString::null, this, i18n( "Global Display Picture" ) );
	if( filePath.isEmpty() )
		return;

	if( !filePath.isLocalFile() ) {
		if(!KIO::NetAccess::download( filePath, path, this )) {
			KMessageBox::sorry( this, i18n( "Downloading of display image failed" ), i18n( "Identity Configuration" ) );
			return;
		}
		remoteFile = true;
	}
	else path = filePath.path();

	QString futurName = locateLocal( "appdata", "global-displayphoto.png");

	QImage img( path );

	if(!img.isNull()) {
		img = img.smoothScale( 96, 96, QImage::ScaleMax );
		// crop image if not square
		if(img.width() > img.height()) {
			img = img.copy((img.width()-img.height())/2, 0, img.height(), img.height());
		}
		else if(img.height() > img.width()) {
			img = img.copy(0, (img.height()-img.width())/2, img.width(), img.width());
		}
		/*if(img.width() > img.height())
		{
			img = img.smoothScale(img.height(), img.height(), QImage::ScaleMax);
			img = img.copy(0, 0, img.height(), img.height());
		}
		else if(img.height() > img.width())
		{
			img = img.smoothScale(img.width(), img.width(), QImage::ScaleMax);
			img = img.copy(0,0,img.width(), img.width());
		}*/

		if ( img.save( futurName, "PNG" ) )
		{
			m_view->m_displayPicture->setPixmap( futurName );
		}
		else
		{
			KMessageBox::sorry( this, i18n( "<qt>An error occurred when trying to change the display picture.<br>"
				"Make sure that you have selected a correct image file</qt>" ), i18n( "Identity Configuration" ) );
		}
	}
	else
	{
		KMessageBox::sorry( this, i18n( "<qt>An error occurred when trying to change the display picture.<br>"
			"Make sure that you have selected a correct image file</qt>" ), i18n( "Identity Configuration" ) );
	}
	if( remoteFile ) KIO::NetAccess::removeTempFile( path );

	emit changed(true);
}

void KopeteIdentityConfig::slotTextChanged(const QString &)
{
	emit changed(true);
}

void KopeteIdentityConfig::slotSettingsChanged()
{
	emit changed(true);
}

void KopeteIdentityConfig::slotComboActivated(int)
{
	emit changed(true);
}

#include "kopeteidentityconfig.moc"
