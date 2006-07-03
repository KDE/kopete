/*
    msndebugrawcmddlg.cpp - Send a raw MSN command for debugging

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>
    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    Portions of this code are taken from KMerlin,
              (c) 2001 by Olaf Lueg              <olueg@olsd.de>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "msndebugrawcmddlg.h"

#include "ui_msndebugrawcommand_base.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <ktextedit.h>

#include <klocale.h>

MSNDebugRawCmdDlg::MSNDebugRawCmdDlg( QWidget *parent )
: KDialog( parent )
{
	setCaption( i18n( "DEBUG: Send Raw Command - MSN Plugin" ) );
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );
	showButtonSeparator( true );

	setInitialSize( QSize( 350, 200 ) );

	QWidget* w = new QWidget( this );
	m_main = new Ui::MSNDebugRawCommand_base();
	m_main->setupUi( w );
	setMainWidget( w );
}

MSNDebugRawCmdDlg::~MSNDebugRawCmdDlg()
{
	m_main;
}

QString MSNDebugRawCmdDlg::command()
{
	return m_main->m_command->text();
}

QString MSNDebugRawCmdDlg::params()
{
	return m_main->m_params->text();
}

bool MSNDebugRawCmdDlg::addNewline()
{
	return m_main->m_addNewline->isChecked();
}

bool MSNDebugRawCmdDlg::addId()
{
	return m_main->m_addId->isChecked();
}

QString MSNDebugRawCmdDlg::msg()
{
	return m_main->m_msg->text();
}

#include "msndebugrawcmddlg.moc"

// vim: set noet ts=4 sts=4 sw=4:

