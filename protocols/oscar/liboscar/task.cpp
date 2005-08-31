/*
    task.cpp - Kopete Groupwise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com

    Based on Iris, Copyright (C) 2003  Justin Karneges

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

#include <QTimer>

#include "connection.h"
#include "transfer.h"
#include "safedelete.h"
#include "buffer.h"
#include "task.h"


class Task::TaskPrivate
{
public:
	TaskPrivate() {}

	quint32 id;
	bool success;
	int statusCode;
	QString statusString;
	Connection* client;
	bool insignificant, deleteme, autoDelete;
	bool done;
	Transfer* transfer;
};

Task::Task(Task *parent)
:QObject(parent)
{
	init();
	d->client = parent->client();
	connect(d->client, SIGNAL(disconnected()), SLOT(clientDisconnected()));
}

Task::Task(Connection* parent, bool)
:QObject(0)
{
	init();
	d->client = parent;
	connect(d->client, SIGNAL(disconnected()), SLOT(clientDisconnected()));
}

Task::~Task()
{
	delete d->transfer;
	delete d;
}

void Task::init()
{
	d = new TaskPrivate;
	d->success = false;
	d->insignificant = false;
	d->deleteme = false;
	d->autoDelete = false;
	d->done = false;
	d->transfer = 0;
	d->id = 0;
}

Task *Task::parent() const
{
	return (Task *)QObject::parent();
}

Connection *Task::client() const
{
	return d->client;
}

Transfer * Task::transfer() const
{
	return d->transfer;
}

void Task::setTransfer( Transfer * transfer )
{
	d->transfer = transfer;
}

long Task::id() const
{
	return d->id;
}

bool Task::success() const
{
	return d->success;
}

int Task::statusCode() const
{
	return d->statusCode;
}

QString Task::statusString() const
{
	return d->statusString;
}

void Task::go(bool autoDelete)
{
	d->autoDelete = autoDelete;

	onGo();
}

bool Task::take( Transfer * transfer)
{
    QList<QObject*> p = children();

	// pass along the transfer to our children
    foreach( QObject* o, p )
    {
        Task *t;
        if ( !qobject_cast<Task*>( o ) )
            continue;

        t = static_cast<Task*>(o);

        if(t->take( transfer ))
            return true;
	}

	return false;
}

void Task::safeDelete()
{
	if(d->deleteme)
		return;

	d->deleteme = true;
	if(!d->insignificant)
		SafeDelete::deleteSingle(this);
}

void Task::onGo()
{
	qDebug( "ERROR: calling default NULL onGo() for this task, you should reimplement this!");
}

void Task::onDisconnect()
{
	if(!d->done) {
		d->success = false;
		d->statusCode = ErrDisc;
		d->statusString = tr("Disconnected");

		// delay this so that tasks that react don't block the shutdown
		QTimer::singleShot(0, this, SLOT(done()));
	}
}

void Task::send( Transfer * request )
{
	client()->send( request );
}

void Task::setSuccess(int code, const QString &str)
{
	if(!d->done) {
		d->success = true;
		d->statusCode = code;
		d->statusString = str;
		done();
	}
}

void Task::setError(int code, const QString &str)
{
	if(!d->done) {
		d->success = false;
		d->statusCode = code;
		d->statusString = str;
		done();
	}
}

void Task::done()
{
	debug("Task::done()");
	if(d->done || d->insignificant)
		return;
	d->done = true;

	if(d->deleteme || d->autoDelete)
		d->deleteme = true;

	d->insignificant = true;
	debug("emitting finished");
	finished();
	d->insignificant = false;

	if(d->deleteme)
		SafeDelete::deleteSingle(this);
}

void Task::clientDisconnected()
{
	onDisconnect();
}

Transfer* Task::createTransfer( struct FLAP f, struct SNAC s, Buffer* buffer )
{
	return new SnacTransfer( f, s, buffer );
}

Transfer* Task::createTransfer( struct FLAP f, Buffer* buffer )
{
	return new FlapTransfer( f, buffer );
}

Transfer* Task::createTransfer( Buffer* buffer )
{
	return new Transfer( buffer );
}


void Task::debug(const QString &str)
{
	//black hole
	Q_UNUSED( str );
	//client()->debug(QString("%1: ").arg(className()) + str);
}

bool Task::forMe( const Transfer * transfer ) const
{
	Q_UNUSED( transfer );
	return false;
}

void Task::setId( quint32 id )
{
	d->id = id;
}

#include "task.moc"

//kate: tab-width 4; indent-mode csands;

