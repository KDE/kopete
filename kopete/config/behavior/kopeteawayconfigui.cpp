/*
    kopeteawayconfigui.cpp  -  Kopete Away Config UI

    Copyright (c) 2002      by Chris TenHarmsel       <ctenha56@calvin.edu>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteawayconfigui.h"
#include "kopeteawayconfigui.moc"

#include <qtextedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>

#include <kdeversion.h>
#if KDE_IS_VERSION( 3, 1, 90 )
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>

#include "kopeteaway.h"

KopeteAwayConfigUI::KopeteAwayConfigUI(QWidget *parent) :
	KopeteAwayConfigBaseUI(parent)
{
	connect(btnNew, SIGNAL(clicked()), this, SLOT(newButtonClicked()));
	connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteButtonClicked()));
	connect(btnSave, SIGNAL(clicked()), this, SLOT(saveTextButtonClicked()));
	connect(lstTitles, SIGNAL(selectionChanged()), this, SLOT(titleSelected()));

}

void KopeteAwayConfigUI::newButtonClicked()
{
	bool createNewTitle = false;
#if KDE_IS_VERSION( 3, 1, 90 )
	QString newTitle = KInputDialog::getText(
		i18n("New Away Message"),
		i18n("Enter away message title:"),
		i18n("Title"), &createNewTitle, this);
#else
	QString newTitle = KLineEditDlg::getText(
		i18n("New Away Message"),
		i18n("Enter away message title:"),
		i18n("Title"), &createNewTitle, this);
#endif

	if(createNewTitle)
	{
		KopeteAway::getInstance()->addMessage( newTitle,
			QString::null ); // Add a new empty away message
		updateView();
	}
}

void KopeteAwayConfigUI::deleteButtonClicked()
{
	if (lstTitles->currentItem() == -1) return;

	QListBoxItem *selectedItem = lstTitles->item( lstTitles->currentItem() );
	int retval = KMessageBox::warningYesNo( this,
		i18n( "Delete message '%1'?").arg( selectedItem->text() ),
		i18n( "Delete Message" ) );

	if( retval == KMessageBox::Yes )
	{
		KopeteAway::getInstance()->deleteMessage(selectedItem->text());
		updateView();
	}
}

void KopeteAwayConfigUI::saveTextButtonClicked()
{
	if (lstTitles->currentItem() == -1) return;

	QListBoxItem *selectedItem = lstTitles->item( lstTitles->currentItem() );
	KopeteAway::getInstance()->updateMessage(selectedItem->text(), txtMessage->text());
}

void KopeteAwayConfigUI::titleSelected()
{
	QListBoxItem *selectedItem = lstTitles->item( lstTitles->currentItem() );
	txtMessage->setText(KopeteAway::getInstance()->getMessage( selectedItem->text() ));
}

void KopeteAwayConfigUI::updateView()
{
	KopeteAway *ka = KopeteAway::getInstance();

	lstTitles->clear();
	QStringList titles = ka->getTitles();
	for( QStringList::iterator i = titles.begin(); i != titles.end(); i++ )
		lstTitles->insertItem( ( *i ) ); // Insert the Title into the list

	txtMessage->setText( QString::null );

	mUseAutoAway->setChecked(ka->useAutoAway());
	mAutoAwayTimeout->setValue((int)(ka->autoAwayTimeout()/60));
	mGoAvailable->setChecked(ka->goAvailable());
}
