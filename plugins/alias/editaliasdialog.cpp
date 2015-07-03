/*
 Kopete Alias Plugin

 Copyright (c) 2005 by Matt Rogers <mattr@kde.org>
 Kopete Copyright (c) 2002-2005 by the Kopete Developers <kopete-devel@kde.org>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************

*/

#include "editaliasdialog.h"
#include <qobject.h>
#include <kpushbutton.h>
#include <qwidget.h>
#include <qstring.h>
#include <klineedit.h>


EditAliasDialog::EditAliasDialog( QWidget* parent )
: QDialog( parent )
{
	setupUi( this );
	
	QObject::connect( alias, SIGNAL(textChanged(QString)), this, SLOT(checkButtonsEnabled()) );
	QObject::connect( command, SIGNAL(textChanged(QString)), this, SLOT(checkButtonsEnabled()) );
	QObject::connect( protocolList, SIGNAL(selectionChanged()), this, SLOT(checkButtonsEnabled()) );
        QObject::connect( kPushButton3, SIGNAL(clicked()), this, SLOT(reject()) );
        QObject::connect( addButton, SIGNAL(clicked()), this, SLOT(accept()) );

	checkButtonsEnabled();
}

EditAliasDialog::~EditAliasDialog()
{
}

void EditAliasDialog::checkButtonsEnabled()
{
	if ( !alias->text().isEmpty() && !command->text().isEmpty() && !protocolList->selectedItems().isEmpty() )
		addButton->setEnabled( true );
	else
		addButton->setEnabled( false ) ;
}

#include "editaliasdialog.moc"

// kate: space-indent off; replace-tabs off; tab-width 4; indent-mode csands;
