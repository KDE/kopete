/*
    kopetepasswordwidget.cpp - widget for modifying a KopetePassword

    Copyright (c) 2003 by Richard Smith          <kde@metafoo.co.uk>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetepasswordwidget.h"
#include "kopetepassword.h"

#include <kpassdlg.h>

#include <qcheckbox.h>

struct KopetePasswordWidget::KopetePasswordWidgetPrivate
{
	uint maxLength;
};

KopetePasswordWidget::KopetePasswordWidget( QWidget *parent, KopetePassword *from, const char *name )
 : KopetePasswordWidgetBase( parent, name ), d( new KopetePasswordWidgetPrivate )
{
	load( from );
}

KopetePasswordWidget::~KopetePasswordWidget()
{
	delete d;
}

void KopetePasswordWidget::load( KopetePassword *source )
{
	if ( source && source->remembered() )
	{
		mRemembered->setTristate();
		mRemembered->setNoChange();
		source->requestWithoutPrompt( this, SLOT( receivePassword( const QString & ) ) );
	}
	else
	{
		mRemembered->setTristate( false );
		mRemembered->setChecked( false );
	}

	if ( source )
		d->maxLength = source->maximumLength();
	else
		d->maxLength = 0;

	mPassword->setEnabled( false );
	connect( mRemembered, SIGNAL( stateChanged( int ) ), SLOT( slotRememberChanged() ) );
	connect( mPassword, SIGNAL( textChanged( const QString & ) ), SIGNAL( changed() ) );
	connect( mRemembered, SIGNAL( stateChanged( int ) ), SIGNAL( changed() ) );
}

void KopetePasswordWidget::slotRememberChanged()
{
	mRemembered->setTristate( false );
	mPassword->setEnabled( mRemembered->isChecked() );
}

void KopetePasswordWidget::receivePassword( const QString &pwd )
{
	// pwd == null could mean user declined to open wallet
	// don't uncheck the remembered field in this case.
	if ( !pwd.isNull() && mRemembered->state() == QButton::NoChange )
	{
		mRemembered->setTristate( false );
		mRemembered->setChecked( true );
		mPassword->clear();
		mPassword->insert( pwd );
		mPassword->setEnabled( true );
	}
}

void KopetePasswordWidget::save( KopetePassword *target )
{
	if ( !target || mRemembered->state() == QButton::NoChange )
		return;
	
	if ( mRemembered->isChecked() )
		target->set( QString::fromLocal8Bit( mPassword->password() ) );
	else
		target->set();
}

bool KopetePasswordWidget::validate()
{
	if ( !mRemembered->isChecked() ) return true;
	if ( d->maxLength == 0 ) return true;
	return QString::fromLocal8Bit( mPassword->password() ).length() <= d->maxLength;
}

#include "kopetepasswordwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:
