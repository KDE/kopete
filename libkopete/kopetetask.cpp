/*
    kopetetask.cpp - Kopete Task

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetetask.h"

#include <klocale.h>

#include <qptrlist.h>

namespace Kopete
{

class Task::Private
{
public:
	Private()
	 : result( ResultFailed )
	{
		errorMessage = i18n( "The operation has not finished yet" );
	}

	Task::Result result;
	QString errorMessage;
	QPtrList<Task> subtasks;
};

Task::Task()
 : d( new Private )
{
}

Task::~Task()
{
	delete d;
}

bool Task::succeeded() const
{
	return d->result == ResultSucceeded;
}

const QString &Task::errorString() const
{
	return d->errorMessage;
}

void Task::abort( int flags )
{
	int childFlags = flags & ~AbortEmitResult;
	for ( Task *task = d->subtasks.first(); task; task = d->subtasks.next() )
		task->abort( childFlags );

	if ( flags & AbortEmitResult )
		emitResult( ResultFailed, i18n( "Aborted" ) );
	else
		delete this;
}

void Task::addSubtask( Task *task )
{
	d->subtasks.append( task );
	connect( task, SIGNAL( result( Kopete::Task* ) ),
	         this, SLOT( slotResult( Kopete::Task* ) ) );
	connect( task, SIGNAL( statusMessage( Kopete::Task*, const QString & ) ),
	         this, SIGNAL( statusMessage( Kopete::Task*, const QString & ) ) );
}

void Task::removeSubtask( Task *task, RemoveSubtaskIfLast actionIfLast )
{
	disconnect( task, SIGNAL( result( Kopete::Task* ) ),
	            this, SLOT( slotResult( Kopete::Task* ) ) );
	disconnect( task, SIGNAL( statusMessage( Kopete::Task*, const QString & ) ),
	            this, SIGNAL( statusMessage( Kopete::Task*, const QString & ) ) );
	d->subtasks.remove( task );
	if ( d->subtasks.isEmpty() && actionIfLast == IfLastEmitResult )
		emitResult( task->succeeded() ? ResultSucceeded : ResultFailed, task->errorString() );
}

void Task::emitResult( Result res, const QString &errorMessage )
{
	d->result = res;
	d->errorMessage = errorMessage;
	emit result( this );
	delete this;
}

void Task::slotResult( Kopete::Task *task )
{
	removeSubtask( task );
}

}

#include "kopetetask.moc"
// vim: set noet ts=4 sts=4 sw=4:
