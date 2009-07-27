/*
    kopetepasswordwidget.cpp - widget for editing a Kopete::Password

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

#ifndef KOPETEPASSWORDWIDGET_H
#define KOPETEPASSWORDWIDGET_H

#include "ui_kopetepasswordwidgetbase.h"
#include "kopete_export.h"

namespace Kopete
{

class Password;
class Protocol;

namespace UI
{

/**
 * @author Richard Smith <kde@metafoo.co.uk>
 * This widget displays an editable password, including the Remember password checkbox.
 * @todo This is NOT BC yet : it derives from a uic-generated class
 */
class KOPETE_EXPORT PasswordWidget : public QWidget, public Ui::KopetePasswordWidgetBase
{
	Q_OBJECT

public:
    PasswordWidget( QWidget *parent = 0 );

	/**
	 * Creates a Kopete::PasswordWidget.
	 * @param from The password to load the data for this widget from, or 0 if none
	 * @param parent The widget to nest this one inside
	 */
	explicit PasswordWidget( Kopete::Password *from, QWidget *parent = 0 );

	~PasswordWidget();

	/**
	 * Sets a protocol to use to validate entered passwords.  If no protocol is set 
	 * validate() always returns true.
	 * @see Kopete::Protocol::validatePassword()
	 * @param proto the protocol to use to validate entered passwords
	 */
	void setValidationProtocol( Kopete::Protocol * );

	/**
	 * Loads the information stored in source into the widget
	 */
	void load( Kopete::Password *source );
	/**
	 * Saves the information in the widget into target
	 */
	void save( Kopete::Password *target );

	/**
	 * Returns true if the information in the widget is valid, false if it is not.
	 * Currently the only way this can fail is if the password is too long.
	 * @todo this should return an enum of failures.
	 */
	bool validate();

	/**
	 * Returns the string currently in the input box in the widget
	 */
	QString password() const;
	/**
	 * Returns a boolean indicating whether the Remember Password checkbox is checked.
	 * Result is undefined if the Remember Password field is in the 'no change' state
	 * because the user has not (yet) opened the wallet.
	 */
	bool remember() const;
	
	/**
	 * Set the password stored in the widget.
	 * @param pass The text to place in the password field.
	 */
	void setPassword( const QString &pass );

signals:
	/**
	 * Emitted when the information stored in this widget changes
	 */
	void changed();

public slots:
	/** @internal */
	void receivePassword( const QString & );

private slots:
	void slotRememberChanged();
	void passwordTextChanged();

private:
	class Private;
	Private * const d;
};

}

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

