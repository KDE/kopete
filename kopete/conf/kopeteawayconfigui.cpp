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

#include <qstringlist.h>
#include <qtextedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>

#include <klineeditdlg.h>
#include <klistbox.h>
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
	QString newTitle = KLineEditDlg::getText(i18n("New Away Message"), i18n("Away Message Title"),
						i18n("Title"), &createNewTitle, this);
	if( createNewTitle )
	{
		KopeteAway::getInstance()->addMessage( newTitle, QString::null ); // Add a new empty away message
		updateView();
	}
}

void KopeteAwayConfigUI::deleteButtonClicked()
{
	if (lstTitles->currentItem() == -1) return;

	QListBoxItem *selectedItem = lstTitles->item( lstTitles->currentItem() );
	int retval = KMessageBox::warningYesNo( this,
		i18n( "Delete Message %1?").arg( selectedItem->text() ),
		i18n( "Delete Message - Kopete" ) );

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
	/* Clear the contents of the listbox */
	lstTitles->clear();
	/* Get all the titles of away messages */
	QStringList titles = KopeteAway::getInstance()->getTitles();
	/* For every title... */
	for( QStringList::iterator i = titles.begin(); i != titles.end(); i++ )
		lstTitles->insertItem( ( *i ) ); // Insert the Title into the list

	txtMessage->setText( QString::null );
	
	mAwayTimeout->setValue((int)(KopeteAway::getInstance()->autoAwayTimeout()/60));
	mGoAvailable->setChecked(KopeteAway::getInstance()->goAvailable());
}
