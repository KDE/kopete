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

#include <qtextedit.h>

#include <klineedit.h>
#include <kconfig.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

//#include "kopetetransfermanager.h"
#include "fileconfirmbase.h"
#include "kopetefileconfirmdialog.h"

#include "kopetemetacontact.h"
#include "kopetecontact.h"

KopeteFileConfirmDialog::KopeteFileConfirmDialog(const Kopete::FileTransferInfo &info,const QString& description,QWidget *parent, const char *name )
: KDialogBase( parent, name, false, i18n( "A User Would Like to Send You a File" ),
	KDialogBase::User1 | KDialogBase::User2, KDialogBase::User1, true, i18n( "&Refuse" ), i18n( "&Accept" ) ),
	m_info( info )
{
	setWFlags( WDestructiveClose );
	m_emited=false;

	m_view=new FileConfirmBase(this, "FileConfirmView");
	m_view->m_from->setText( info.contact()->metaContact()->displayName() + QString::fromLatin1( " <" ) +
			info.contact()->contactId() + QString::fromLatin1( "> " ) );
	m_view->m_size->setText( KGlobal::locale()->formatNumber( long( info.size() ), 0 ) );
	m_view->m_description->setText( description );
	m_view->m_filename->setText( info.file() );
	
	KGlobal::config()->setGroup("File Transfer");
	const QString defaultPath=KGlobal::config()->readEntry("defaultPath" , QDir::homeDirPath() );
	m_view->m_saveto->setText(defaultPath  + QString::fromLatin1( "/" ) + info.file() );

	setMainWidget(m_view);

	connect(m_view->cmdBrowse, SIGNAL(clicked()), this, SLOT(slotBrowsePressed()));
}

KopeteFileConfirmDialog::~KopeteFileConfirmDialog()
{
}

void KopeteFileConfirmDialog::slotBrowsePressed()
{
	QString saveFileName = KFileDialog::getSaveFileName( m_view->m_saveto->text(), QString::fromLatin1( "*" ), 0L , i18n( "File Transfer" ) );
	if ( !saveFileName.isNull())
	{
		m_view->m_saveto->setText(saveFileName);
	}
}

void KopeteFileConfirmDialog::slotUser2()
{
	m_emited=true;
	KURL url(m_view->m_saveto->text());
	if(url.isValid() && url.isLocalFile() )
	{
		const QString directory=url.directory();
		if(!directory.isEmpty())
		{
			KGlobal::config()->setGroup("File Transfer");
			KGlobal::config()->writeEntry("defaultPath" , directory );
		}
		
		if(QFile(m_view->m_saveto->text()).exists())
		{
			int ret=KMessageBox::warningContinueCancel(this,  i18n("The file '%1' already exists.\nDo you want to overwrite it ?").arg(m_view->m_saveto->text()) ,
					 i18n("Overwrite File") , KStdGuiItem::save());
			if(ret==KMessageBox::Cancel)
				return;
		}
	 
		emit accepted(m_info,m_view->m_saveto->text());
		close();
	}
	else
		KMessageBox::queuedMessageBox (this, KMessageBox::Sorry, i18n("You must provide a valid local filename") );
}

void KopeteFileConfirmDialog::slotUser1()
{
	m_emited=true;
	emit refused(m_info);
	close();
}

void KopeteFileConfirmDialog::closeEvent( QCloseEvent *e)
{
	if(!m_emited)
	{
		m_emited=true;
		emit refused(m_info);
	}
	KDialogBase::closeEvent(e);
}

#include "kopetefileconfirmdialog.moc"

