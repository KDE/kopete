/*
  kopeteawaydialog.cpp  -  Kopete Away Dialog

  Copyright (c) 2002 by Hendrik vom Lehn <hvl@linux-4-ever.de>

  Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#include "kopeteawaydialog.h"

#include <qpushbutton.h>

#include <kcombobox.h>
#include <klineedit.h>
#include <kdebug.h>

#include "kopeteaway.h"

KopeteAwayDialog::KopeteAwayDialog(QWidget *parent, const char *name)
				: KopeteAwayBase(parent, name)
{
//	kdDebug(14011) << "[KopeteAwayDialog] Building KopeteAwayDialog..." << endl;

	// Connect the buttons to actions
	QObject::connect( cmdCancel, SIGNAL(clicked()),
		this, SLOT(slotCancelClicked()) );
	QObject::connect( cmdOkay, SIGNAL(clicked()),
		this, SLOT(slotOkayClicked()) );

	// Connect the user input widgets with their slots
	// These objects are inherited from KopeteAwayBase
	// Which is built using QT Designer
	QObject::connect( cmbHistory, SIGNAL(activated(int)),
		this, SLOT(slotComboBoxSelection(int)));

	// Get the KopeteAway instance
	awayInstance = KopeteAway::getInstance();

	// The last user entered away message is blank by default
	mLastUserAwayMessage = "";

	// Set the extended away type
	mExtendedAwayType = 0;

	// Call the init method
	init();

	// Set us modal
	setWFlags( Qt::WType_Dialog | Qt::WShowModal );

//	kdDebug(14011) << "[KopeteAwayDialog] KopeteAwayDialog Created." << endl;
}

KopeteAwayDialog::~KopeteAwayDialog(){}

void KopeteAwayDialog::slotComboBoxSelection(int /*index*/)
{
	// If they selected something out of the combo box
	// They probably want to use it
	txtOneShot->setText( awayInstance->getMessage( cmbHistory->currentText() ) );
	txtOneShot->setCursorPosition(0);
}

void KopeteAwayDialog::show()
{
	// When this show is called, set the
	// mExtendedAwayType to the empty string
	mExtendedAwayType = 0;

	// Reinit the GUI
	init();

//	kdDebug(14011) << "[KopeteAwayDialog] Showing Dialog with no " << "extended away type" << endl;

	// Call the parent class' show method
	KopeteAwayBase::show();
}

void KopeteAwayDialog::show(int awayType)
{
	// Save the away message
	mExtendedAwayType = awayType;

	// Reinit the GUI to set it up correctly
	init();

	// Print a debug statement telling what's going on
	kdDebug(14011) << "[KopeteAwayDialog] Showing Dialog with "
		<< "extended away type " << awayType << endl;

	// Call the parent class' show method
	KopeteAwayBase::show();
}

void KopeteAwayDialog::init()
{
	// Clear out the list of titles
	cmbHistory->clear();
	// Insert the string list of titles
	cmbHistory->insertStringList(awayInstance->getTitles());
	// Fill in the text they typed last
	txtOneShot->setText( awayInstance->getMessage( cmbHistory->currentText() ) );
	// Give it the focus so they can just begin
	// typing if they want
	txtOneShot->setFocus();
	txtOneShot->setCursorPosition(0);
}

QString KopeteAwayDialog::getSelectedAwayMessage()
{
	mLastUserAwayMessage = txtOneShot->text();
	return mLastUserAwayMessage;
}

void KopeteAwayDialog::slotOkayClicked()
{
	// Save the text the user typed
	mLastUserTypedMessage = txtOneShot->text();
	// Call the virtual function with the type of away
	setAway(mExtendedAwayType);
	// Close the window
	close();
}

void KopeteAwayDialog::slotCancelClicked()
{
	// Call the virtual function with the type of away
	cancelAway(mExtendedAwayType);
	// Close the window
	close();
}

#include "kopeteawaydialog.moc"
/*
 * Local variables:
 * mode: c++
 * c-indentation-style: k&r
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */

// vim: set noet ts=4 sts=4 sw=4:

