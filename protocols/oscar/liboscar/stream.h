/*
    stream.h - Kopete Groupwise Protocol

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>

    Based on code copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qobject.h>

#ifndef OSCARSTREAM_H
#define OSCARSTREAM_H

class Transfer;

class Stream : public QObject
{
	Q_OBJECT
public:
	Stream( QObject *parent = 0 );
	virtual ~Stream();

	virtual void close() = 0;
	virtual int error() const = 0;
	virtual QString errorString() const = 0;

	/**
	 * Are there any messages waiting to be read
	 */
	virtual bool transfersAvailable() const = 0;	// adapt to messages

	/**
	 * Read a message received from the server
	 */
	virtual Transfer* read() = 0;

	/**
	 * Send a message to the server
	 */
	virtual void write( Transfer *request ) = 0;

signals:
	void disconnected();
	void readyRead(); //signals that there is a transfer ready to be read
	void error( int );
};

#endif
