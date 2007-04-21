/*
    xtrazicqstatusdialog.cpp  -  Xtraz ICQ Status Dialog

    Copyright (c) 2007 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "xtrazicqstatusdialog.h"

#include "ui_xtrazicqstatusui.h"

#include "oscartypes.h"

namespace Xtraz
{

ICQStatusDialog::ICQStatusDialog( QWidget *parent )
: KDialog( parent )
{
	setCaption( i18n( "Set Xtraz Status" ) );
	setButtons( KDialog::Ok | KDialog::Cancel );
	
	mXtrazStatusUI = new Ui::XtrazICQStatusUI();
	QWidget *w = new QWidget( this );
	mXtrazStatusUI->setupUi( w );
	setMainWidget( w );

	QList<QIcon> icons;
	for ( int i = 0; i < Oscar::XSTAT_LAST; ++i )
		icons << KIcon( QString( "icq_xstatus%1" ).arg( i ) );

	mXtrazStatusUI->iconsWidget->setColumnCount( 11 );
	mXtrazStatusUI->iconsWidget->setIcons( icons );
	mXtrazStatusUI->iconsWidget->setSelectedIndex( 0 );
	mXtrazStatusUI->iconsWidget->setTabKeyNavigation( false );
	mXtrazStatusUI->iconsWidget->setFocus();
}

ICQStatusDialog::~ICQStatusDialog()
{
	delete mXtrazStatusUI;
}

void ICQStatusDialog::setXtrazStatus( Xtraz::Status status )
{
	mXtrazStatusUI->iconsWidget->setSelectedIndex( status.status() );
	mXtrazStatusUI->descriptionEdit->setText( status.description() );
	mXtrazStatusUI->messageEdit->setText( status.message() );
}

Xtraz::Status ICQStatusDialog::xtrazStatus() const
{
	Xtraz::Status status;

	status.setStatus( mXtrazStatusUI->iconsWidget->selectedIndex() );
	status.setDescription( mXtrazStatusUI->descriptionEdit->text() );
	status.setMessage( mXtrazStatusUI->messageEdit->text() );

	return status;
}

bool ICQStatusDialog::append() const
{
	return mXtrazStatusUI->checkAppend->isChecked();
}

}
