/*
    task.h - Kopete Oscar Protocol

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

#ifndef OSCAR_TASK_H
#define OSCAR_TASK_H

#include <qobject.h>

#include "oscartypes.h"


class QString;
class Buffer;
class Connection;
class Transfer;

using namespace Oscar;


class Task : public QObject
{
	Q_OBJECT
public:
	enum AutoDeleteSetting { DoNotAutoDelete, AutoDelete };
	enum { ErrDisc };
	Task(Task *parent);
	Task( Connection*, bool isRoot );
	virtual ~Task();

	Task *parent() const;
	Connection* client() const;
	Transfer *transfer() const;

	quint32 id() const;

	bool success() const;
	int statusCode() const;
	const QString & statusString() const;

	void go( AutoDeleteSetting autoDelete = DoNotAutoDelete );

	/**
	 * Allows a task to examine an incoming Transfer and decide whether to 'take' it
	 * for further processing.
	 */
	virtual bool take( Transfer* transfer );
	void safeDelete();

	/**
	 * Direct setter for Tasks which don't have any fields
	 */
	void setTransfer( Transfer * transfer );

signals:
	void finished();

protected:
	virtual void onGo();
	virtual void onDisconnect();
	void setId( quint32 id );
	void send( Transfer * request );
	void setSuccess( int code=0, const QString &str="" );
	void setError( int code=0, const QString &str="" );

	/**
	 * Used in take() to check if the offered transfer is for this Task
	 * @return true if this Task should take the Transfer.  Default impl always returns false.
	 */
	virtual bool forMe( const Transfer * transfer ) const;

	/**
	 * Creates a transfer with the given flap, snac, and buffer
	 */
	Transfer* createTransfer( FLAP f, SNAC s, Buffer* buffer );

	/**
	 * Creates a transfer with the given flap and buffer
	 */
	Transfer* createTransfer( FLAP f, Buffer* buffer );

	/**
	 * Creates a transfer with the given buffer
	 */
	Transfer* createTransfer( Buffer* buffer );

private slots:
	void clientDisconnected();
	void done();

private:
	void init();

	class TaskPrivate;
	TaskPrivate *d;
};

#endif
