/*
   task.cpp - Papillon Task base class.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Based on code Copyright (c) 2004 Matt Rogers <mattr@kde.org>
   Based on code copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
   Based on Iris, Copyright (C) 2003  Justin Karneges

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "Papillon/Task"

#include <QtCore/QTimer>

#include <QtDebug>

// Papillon includes
#include "Papillon/Transfer"
#include "Papillon/Base/SafeDelete"
#include "Papillon/Connection"

namespace Papillon
{

class Task::Private
{
public:
	Private() {}

	bool success;
	int statusCode;
	QString statusString;
	Connection *connection;
	bool insignificant, deleteme, autoDelete;
	bool done;
	Transfer *transfer;
};

Task::Task(Task *parent)
 : QObject(parent)
{
	init();
	d->connection = parent->connection();
	connect(d->connection, SIGNAL(disconnected()), SLOT(connectionDisconnected()));
}

Task::Task(Connection* parent, bool)
 : QObject(0)
{
	init();
	d->connection = parent;
	connect(d->connection, SIGNAL(disconnected()), SLOT(connectionDisconnected()));
}

Task::~Task()
{
	delete d;
}

void Task::init()
{
	d = new Private;
	d->success = false;
	d->insignificant = false;
	d->deleteme = false;
	d->autoDelete = false;
	d->done = false;
	d->transfer = 0;
}

Task *Task::parent() const
{
	return static_cast<Task *>( QObject::parent() );
}

Connection *Task::connection() const
{
	return d->connection;
}

Transfer *Task::transfer() const
{
	return d->transfer;
}

void Task::setTransfer(Transfer *transfer)
{
	d->transfer = transfer;
}

bool Task::success() const
{
	return d->success;
}

int Task::statusCode() const
{
	return d->statusCode;
}

const QString &Task::statusString() const
{
	return d->statusString;
}

void Task::go(GoParameters args)
{
	d->autoDelete = (args & AutoDelete);

	onGo();
}

bool Task::take(Transfer *transfer)
{
	const QList<Task*> taskList = findChildren<Task*>();

	// pass along the transfer to our children
	foreach( Task* task, taskList )
	{
		if ( task->take( transfer ) )
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
	qDebug() << PAPILLON_FUNCINFO << "ERROR: calling default NULL onGo() for this task, you should reimplement this!";
}

void Task::onDisconnect()
{
	if(!d->done) 
	{
		d->success = false;
		d->statusCode = ErrorDisconnected;
		d->statusString = tr("Disconnected");

		// delay this so that tasks that react don't block the shutdown
		QTimer::singleShot(0, this, SLOT(done()));
	}
}

void Task::send(Transfer *request)
{
	connection()->send( request );
}

void Task::setSuccess(int code, const QString &str)
{
	if(!d->done) 
	{
		d->success = true;
		d->statusCode = code;
		d->statusString = str;
		done();
	}
}

void Task::setError(int code, const QString &str)
{
	if(!d->done) 
	{
		d->success = false;
		d->statusCode = code;
		d->statusString = str;
		done();
	}
}

void Task::done()
{
	if(d->done || d->insignificant)
		return;
	d->done = true;

	if(d->deleteme || d->autoDelete)
		d->deleteme = true;

	d->insignificant = true;
	qDebug() << PAPILLON_FUNCINFO << "emitting finished";
	emit finished(this);
	d->insignificant = false;

	if(d->deleteme)
		SafeDelete::deleteSingle(this);
}

void Task::connectionDisconnected()
{
	onDisconnect();
}

void Task::debug(const QString &str)
{
	//black hole
	Q_UNUSED( str );
	//client()->debug(QString("%1: ").arg(className()) + str);
}

bool Task::forMe(Transfer *transfer) const
{
	Q_UNUSED( transfer );
	return false;
}

}
#include "task.moc"

//kate: tab-width 4; indent-mode csands;

