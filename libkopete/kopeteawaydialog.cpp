/*
    kopeteawaydialog.cpp - Kopete Away Dialog

    Copyright (c) 2002      by Hendrik vom Lehn       <hvl@linux-4-ever.de>
    Copyright (c) 2003      by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteawaydialog.h"

#include <kcombobox.h>
#include <kdebug.h>
#include <klineedit.h>
#include <klocale.h>
#include <kstringhandler.h>

#include "kopeteaway.h"
#include "kopeteawaydialogbase.h"

class KopeteAwayDialogPrivate
{
public:
	KopeteAwayDialog_Base *base;
};

KopeteAwayDialog::KopeteAwayDialog( QWidget *parent, const char *name )
: KDialogBase( parent, name, true, i18n( "Global Away Message" ),
	KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true )
{
	//kdDebug( 14010 ) << k_funcinfo << "Building KopeteAwayDialog..." << endl;

	d = new KopeteAwayDialogPrivate;

	d->base = new KopeteAwayDialog_Base( this );
	setMainWidget( d->base );

	QObject::connect( d->base->cmbHistory, SIGNAL( activated( int ) ), this, SLOT( slotComboBoxSelection( int ) ) );

	awayInstance = Kopete::Away::getInstance();
	mExtendedAwayType = 0;
	init();

	//kdDebug( 14010 ) << k_funcinfo << "KopeteAwayDialog created." << endl;
}

KopeteAwayDialog::~KopeteAwayDialog()
{
	delete d;
}

void KopeteAwayDialog::slotComboBoxSelection( int index )
{
	// If they selected something out of the combo box
	// They probably want to use it
	d->base->txtOneShot->setText( awayInstance->getMessage(index) );
	d->base->txtOneShot->setCursorPosition( 0 );
}

void KopeteAwayDialog::show()
{
	// When this show is called, set the
	// mExtendedAwayType to the empty string
	mExtendedAwayType = 0;

	// Reinit the GUI
	init();

	//kdDebug( 14010 ) << k_funcinfo << "Showing Dialog with no extended away type" << endl;

	KDialogBase::show();
}

void KopeteAwayDialog::show( int awayType )
{
	mExtendedAwayType = awayType;

	// Reinit the GUI to set it up correctly
	init();

	kdDebug( 14010 ) << k_funcinfo << "Showing Dialog with extended away type " << awayType << endl;

	KDialogBase::show();
}

void KopeteAwayDialog::cancelAway( int /* awayType */ )
{
	/* Empty default implementation */
}

void KopeteAwayDialog::init()
{
	QStringList awayMessages = awayInstance->getMessages();
	for( QStringList::iterator it = awayMessages.begin(); it != awayMessages.end(); ++it )
	{
		*it = KStringHandler::rsqueeze( *it );
	}

	d->base->cmbHistory->clear();
	d->base->cmbHistory->insertStringList( awayMessages );
	d->base->txtOneShot->setText( awayMessages[0] );

	d->base->txtOneShot->setFocus();
	d->base->txtOneShot->setCursorPosition( 0 );
}

QString KopeteAwayDialog::getSelectedAwayMessage()
{
	mLastUserAwayMessage = d->base->txtOneShot->text();
	return mLastUserAwayMessage;
}

void KopeteAwayDialog::slotOk()
{
	// Save the text the user typed
	mLastUserTypedMessage = d->base->txtOneShot->text();

	setAway( mExtendedAwayType );

	KDialogBase::slotOk();
}

void KopeteAwayDialog::slotCancel()
{
	// Call the virtual function with the type of away
	cancelAway( mExtendedAwayType );

	KDialogBase::slotCancel();
}

#include "kopeteawaydialog.moc"

// vim: set noet ts=4 sts=4 sw=4:

