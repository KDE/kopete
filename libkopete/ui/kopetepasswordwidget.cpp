/*
    kopetepasswordwidget.cpp - widget for modifying a Kopete::Password

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
#include "kopeteprotocol.h"

#include <klineedit.h>

#include <qcheckbox.h>

namespace Kopete
{
	namespace UI
	{
		class PasswordWidget::Private
		{
			public:
				Private() : protocol( 0 ) { }
				Kopete::Protocol * protocol;
		};
	} // namespace UI
} // namespace Kopete

Kopete::UI::PasswordWidget::PasswordWidget( QWidget *parent )
	    : QWidget( parent ), d( new Private )
{
	setupUi( this );
	mPassword->setPasswordMode(true);
}

Kopete::UI::PasswordWidget::PasswordWidget( Kopete::Password *from, QWidget *parent )
	: QWidget( parent ), d( new Private )
{
	setupUi( this );
	mPassword->setPasswordMode(true);

	load( from );
}

Kopete::UI::PasswordWidget::~PasswordWidget()
{
	delete d;
}

void Kopete::UI::PasswordWidget::setValidationProtocol( Kopete::Protocol * proto )
{
	d->protocol = proto;
}

void Kopete::UI::PasswordWidget::load( Kopete::Password *source )
{
	disconnect( mRemembered, SIGNAL(stateChanged(int)), this, SLOT(slotRememberChanged()) );
	disconnect( mPassword, SIGNAL(textChanged(QString)),
			this, SLOT(passwordTextChanged()) );
	disconnect( mRemembered, SIGNAL(stateChanged(int)), this, SIGNAL(changed()) );

	if ( source && source->remembered() )
	{
		mRemembered->setTristate();
		mRemembered->setCheckState( Qt::PartiallyChecked );
		mPassword->setEnabled( true );
		source->requestWithoutPrompt( this, SLOT(receivePassword(QString)) );
	}
	else
	{
		mRemembered->setTristate( false );
		mRemembered->setCheckState( Qt::Unchecked );
		mPassword->setEnabled( false );
	}

	connect( mRemembered, SIGNAL(stateChanged(int)), this, SLOT(slotRememberChanged()) );
	connect( mPassword, SIGNAL(textChanged(QString)),
			this, SLOT(passwordTextChanged()) );
	connect( mRemembered, SIGNAL(stateChanged(int)), this, SIGNAL(changed()) );

	emit changed();
}

void Kopete::UI::PasswordWidget::slotRememberChanged()
{
	mRemembered->setTristate( false );
	mPassword->setEnabled( mRemembered->isChecked() );
}

void Kopete::UI::PasswordWidget::receivePassword( const QString &pwd )
{
	// pwd == null could mean user declined to open wallet
	// don't uncheck the remembered field in this case.
	if ( !pwd.isNull() && mRemembered->checkState() == Qt::PartiallyChecked )
	{
		mRemembered->setChecked( true );
		setPassword( pwd );
	}
}

void Kopete::UI::PasswordWidget::save( Kopete::Password *target )
{
	if ( !target || mRemembered->checkState() == Qt::PartiallyChecked )
		return;

	if ( mRemembered->isChecked() )
		target->set( password() );
	else
		target->set();
}

bool Kopete::UI::PasswordWidget::validate()
{
	// Unchecked means the password should not be remembered, partially checked 
	// we're waiting for kwallet. Let it pass in both cases.
	if ( mRemembered->checkState() != Qt::Checked )
		return true;

	if ( d->protocol ) {
		return d->protocol->validatePassword( password() );
	}
	return true;
}

QString Kopete::UI::PasswordWidget::password() const
{
	return mPassword->text();
}

bool Kopete::UI::PasswordWidget::remember() const
{
	return mRemembered->checkState() == Qt::Checked;
}

void Kopete::UI::PasswordWidget::setPassword( const QString &pass )
{
	// switch out of 'waiting for wallet' mode if we're in it
	mRemembered->setTristate( false );

	// fill in the password text
	mPassword->clear();
	mPassword->setText( pass );
	mPassword->setEnabled( remember() );
}

void Kopete::UI::PasswordWidget::passwordTextChanged()
{
	if ( mRemembered->checkState() == Qt::PartiallyChecked )
	{
		disconnect( mRemembered, SIGNAL(stateChanged(int)), this, SIGNAL(changed()) );
		// switch out of 'waiting for wallet' mode if we're in it
		mRemembered->setTristate( false );
		mRemembered->setChecked(true);
		connect( mRemembered, SIGNAL(stateChanged(int)), this, SIGNAL(changed()) );
	}
	emit changed();
}

#include "kopetepasswordwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:
