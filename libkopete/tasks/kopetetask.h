/*
    kopetetask.h - Kopete Task

    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETETASK_H
#define KOPETETASK_H

#include "kopete_export.h"
#include <kcompositejob.h>

namespace Kopete
{

/**
 * @brief Base class for all Kopete task.
 *
 * A Task is an encapsulation of a set of commands and data
 * that perform a given task. In our case, a task can contain
 * one or many sub tasks that will help the parent task to do
 * its job.
 *
 * For implementers, you should let the sub tasks do their job
 * before doing something in the parent task. Like for example
 * a sub task will modify the server list contact list or other
 * things.
 *
 * It is a really good idea to always call base start() method because
 * it execute all sub tasks.
 *
 * Some sub tasks are specific to a protocol and they are created
 * using the factory method Kopete::Protocol::createProtocolTask().
 * See the documentation of this method for more information.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETE_EXPORT Task : public KCompositeJob
{
	Q_OBJECT
public:
	/**
	 * Common error code for Kopete tasks
	 */
	enum KopeteTaskError
	{
		/**
		 * Forgot to add the protocol subtask for deleting the contact
		 */
		NoProtocolSubTaskError = KJob::UserDefinedError+1,
		/**
		 * The network is unavailable and thus cannot delete the contact
		 */
		NetworkUnavailableError
    };

	/**
	 * @brief Create a new task
	 * @param parent QObject parent
	 */
	Task(QObject *parent = 0);
	/**
	 * Destructor
	 */
	virtual ~Task();

	/**
	 * @brief Add a new sub task
	 * @param task a new task
	 */
	void addSubTask(KJob *task);

	/**
	 * @brief Execute the task
	 *
	 * The default behavior for the start() here
	 * is to execute all subjobs if they are available.
	 *
	 * For definition of classes in libkopete, start()
	 * should implement the default behavior for the kind
	 * of task.
	 *
	 * For example, for DeleteContactTask, the default behavior
	 * should call deleteLater() on contact instance.
	 */
	virtual void start();

private:
	class Private;
	Private * const d;
};

}

#endif
