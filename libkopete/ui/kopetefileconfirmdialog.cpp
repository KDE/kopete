/***************************************************************************
                          kopetefileconfirmdialog.cpp  -  description
                             -------------------
    begin                : dim nov 17 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qtextedit.h>
 
#include <klineedit.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kpushbutton.h>

//#include "kopetetransfermanager.h" 
#include "fileconfirmbase.h"
#include "kopetefileconfirmdialog.h"

#include "kopetecontact.h"
#include "kopetemetacontact.h"

KopeteFileConfirmDialog::KopeteFileConfirmDialog(const KopeteFileTransferInfo &info,const QString& description,QWidget *parent, const char *name )
	: KDialogBase(parent,name, true, i18n("A User Would Like to Send You a File") , KDialogBase::User1|KDialogBase::User2, KDialogBase::User1, true, i18n("&Accept"), i18n("&Refuse"))
{
	m_emited=false;
	m_info=KopeteFileTransferInfo(info);

	m_view=new FileConfirmBase(this, "FileConfirmView");
	m_view->m_from->setText( info.contact()->metaContact()->displayName() + " <"+info.contact()->id()+"> ");
	m_view->m_size->setText( QString::number(info.size()) );
	m_view->m_description->setText( description );
	m_view->m_filename->setText( info.file() );
	m_view->m_saveto->setText( QDir::homeDirPath()+"/"+info.file() );

	setMainWidget(m_view);

	connect(m_view->cmdBrowse, SIGNAL(pressed()), this, SLOT(slotBrowsePressed()));
	
}

KopeteFileConfirmDialog::~KopeteFileConfirmDialog()
{
}


void KopeteFileConfirmDialog::slotBrowsePressed()
{
	QString saveFileName = KFileDialog::getSaveFileName( m_view->m_saveto->text() ,"*.*", 0l  , i18n( "Kopete File Transfer" ) );
	if ( !saveFileName.isNull())
	{
		m_view->m_saveto->setText(saveFileName);
	}
}

void KopeteFileConfirmDialog::slotUser1()
{
	m_emited=true;
	emit accepted(m_info,m_view->m_saveto->text());
	close();
}

void KopeteFileConfirmDialog::slotUser2()
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

