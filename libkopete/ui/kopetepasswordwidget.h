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
 */
class KopetePasswordWidget : public KopetePasswordWidgetBase
{
	Q_OBJECT

public:
	KopetePasswordWidget( QWidget *parent, KopetePassword *from = 0, const char *name = 0 );
	~KopetePasswordWidget();

	void load( KopetePassword *source );
	void save( KopetePassword *target );

	bool validate( int maxLength = 0 );

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

