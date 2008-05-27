/*
    kopetetask.cpp - Kopete Task

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>

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

#include <kdebug.h>

namespace Kopete
{

class Task::Private
{
public:
};

Task::Task(QObject *parent)
 : KCompositeJob(parent), d( new Private )
{
}

Task::~Task()
{
	delete d;
}

void Task::addSubTask(KJob *task)
{
	addSubjob(task);
}

void Task::start()
{
	kDebug(14010) << "Executing children tasks for this task.";
	KJob *subTask = 0;
	foreach( subTask, subjobs() )
	{
		subTask->start();
	}
}

}

#include "kopetetask.moc"
