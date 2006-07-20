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
#include <kdebug.h>

#include <QList>

namespace Kopete
{

class Task::Private
{
public:

	QList<Task*> subtasks;
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
	return error()==0;
}

void Task::abort( int flags )
{
	int childFlags = flags & ~AbortEmitResult;
	QList<Task*>::iterator it, itEnd = d->subtasks.end();
	for ( it = d->subtasks.begin(); it != itEnd; ++it )
		( *it )->abort( childFlags );

	if ( flags & AbortEmitResult )
		emitTaskResult( ResultFailed, i18n( "Aborted" ) );
	else
		delete this;
}

void Task::addSubtask( Task *task )
{
	d->subtasks.append( task );
	connect( task, SIGNAL( result( KJob* ) ),
	         this, SLOT( slotResult( KJob* ) ) );
	connect( task, SIGNAL( statusMessage( Kopete::Task*, const QString & ) ),
	         this, SIGNAL( statusMessage( Kopete::Task*, const QString & ) ) );
}

void Task::removeSubtask( Task *task, RemoveSubtaskIfLast actionIfLast )
{
	disconnect( task, SIGNAL( result( KJob* ) ),
	            this, SLOT( slotResult( KJob* ) ) );
	disconnect( task, SIGNAL( statusMessage( Kopete::Task*, const QString & ) ),
	            this, SIGNAL( statusMessage( Kopete::Task*, const QString & ) ) );
	d->subtasks.removeAll( task );
	if ( d->subtasks.isEmpty() && actionIfLast == IfLastEmitResult )
		emitTaskResult( task->succeeded() ? ResultSucceeded : ResultFailed, task->errorString() );
}

void Task::setResult( Result res )
{
	setError(res);
}

void Task::emitTaskResult( Result res, const QString &errorMessage )
{
	setResult( res );
	setErrorText(errorMessage);
	emitResult();
}

void Task::slotResult( KJob *task )
{
	removeSubtask( static_cast<Task*>(task) );
}

}

#include "kopetetask.moc"
// vim: set noet ts=4 sts=4 sw=4:
