/*
    kopetefileconfirmdialog.cpp

    Copyright (c) 2003-2004 by Olivier Goffart       <ogoffart @ kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <QCloseEvent>

#include <klineedit.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kguiitem.h>

//#include "kopetetransfermanager.h"
#include "kopetefileconfirmdialog.h"

#include "kopetemetacontact.h"
#include "kopetecontact.h"

KopeteFileConfirmDialog::KopeteFileConfirmDialog(const Kopete::FileTransferInfo &info,const QString& description,QWidget *parent )
: KDialog( parent ), m_info( info )
{
	setCaption( i18n( "A User Would Like to Send You a File" ) );
	setButtons( KDialog::Ok | KDialog::Cancel );
	setButtonGuiItem( KDialog::Cancel, KGuiItem( i18n("&Refuse") ) );
	setButtonGuiItem( KDialog::Ok, KGuiItem( i18n("&Accept") ) );

	setAttribute( Qt::WA_DeleteOnClose );
	m_emited=false;

	m_view=new QWidget( this );
	m_view->setObjectName( "FileConfirmView" );
	setupUi( m_view );

	m_from->setText( info.contact()->metaContact()->displayName() + QLatin1String( " <" ) +
			info.contact()->contactId() + QLatin1String( "> " ) );
	m_size->setText( KGlobal::locale()->formatNumber( long( info.size() ), 0 ) );
	m_description->setText( description );
	m_filename->setText( info.file() );
	if( !info.preview().isNull() )
		m_preview->setPixmap( info.preview() );
	else
		m_preview->setVisible( false );

	KGlobal::config()->setGroup("File Transfer");
	const QString defaultPath=KGlobal::config()->readEntry("defaultPath" , QDir::homePath() );
	m_saveto->setText(defaultPath  + QLatin1String( "/" ) + info.file() );

	setMainWidget(m_view);

	connect(cmdBrowse, SIGNAL(clicked()), this, SLOT(slotBrowsePressed()));
	connect(this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()));
	connect(this, SIGNAL(user2Clicked()), this, SLOT(slotUser2()));
}

KopeteFileConfirmDialog::~KopeteFileConfirmDialog()
{
	if(!m_emited)
	{
		emit refused(m_info);
	}
}

void KopeteFileConfirmDialog::slotBrowsePressed()
{
	QString saveFileName = KFileDialog::getSaveFileName( m_saveto->text(), QLatin1String( "*" ), 0L , i18n( "File Transfer" ) );
	if ( !saveFileName.isNull())
	{
		m_saveto->setText(saveFileName);
	}
}

void KopeteFileConfirmDialog::accept()
{
	KUrl url = KUrl(m_saveto->text());
	if(url.isValid() && url.isLocalFile() )
	{
		const QString directory=url.directory();
		if(!directory.isEmpty())
		{
			KGlobal::config()->setGroup("File Transfer");
			KGlobal::config()->writeEntry("defaultPath" , directory );
		}

		if(QFile(m_saveto->text()).exists())
		{
			int ret=KMessageBox::warningContinueCancel(this, i18n("The file '%1' already exists.\nDo you want to overwrite it ?", m_saveto->text()) ,
					 i18n("Overwrite File") , KStandardGuiItem::save());
			if(ret==KMessageBox::Cancel)
				return;
		}

		emit accepted(m_info,m_saveto->text());
		m_emited=true;
		KDialog::accept();
	}
	else
		KMessageBox::queuedMessageBox (this, KMessageBox::Sorry, i18n("You must provide a valid local filename") );
}

#include "kopetefileconfirmdialog.moc"

