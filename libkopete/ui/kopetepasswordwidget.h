/*
    kopetepasswordwidget.cpp - widget for editing a KopetePassword

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

#include "kopetepasswordwidgetbase.h"

class KopetePassword;

/**
 * @author Richard Smith <kde@metafoo.co.uk>
 * This widget displays an editable password, including the Remember password checkbox.
 */
class KopetePasswordWidget : public KopetePasswordWidgetBase
{
	Q_OBJECT

public:
	/**
	 * Creates a KopetePasswordWidget.
	 * @param parent The widget to nest this one inside
	 * @param from The password to load the data for this widget from, or 0 if none
	 * @param name The name of this QObject
	 */
	KopetePasswordWidget( QWidget *parent, KopetePassword *from = 0, const char *name = 0 );
	~KopetePasswordWidget();

	/**
	 * Loads the information stored in source into the widget
	 */
	void load( KopetePassword *source );
	/**
	 * Saves the information in the widget into target
	 */
	void save( KopetePassword *target );

	/**
	 * Returns true if the information in the widget is valid, false if it is not.
	 * Currently the only way this can fail is if the password is too long.
	 * @todo this should return an enum of failures.
	 */
	bool validate();

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

private:
	struct KopetePasswordWidgetPrivate;
	KopetePasswordWidgetPrivate *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

