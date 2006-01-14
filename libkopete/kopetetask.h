/*
    kopetetask.h - Kopete Task

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

#ifndef KOPETETASK_H
#define KOPETETASK_H

#include <qobject.h>
#include <kdemacros.h>

namespace Kopete
{

/**
 * The base class for all tasks.
 * For most tasks created in Kopete, the code looks like
 *
 * \code
 *   Kopete::Task *task = someobject->someoperation( some parameters );
 *   connect( task, SIGNAL( result( Kopete::Task * ) ),
 *            this, SLOT( slotResult( Kopete::Task * ) ) );
 * \endcode
 *   (other connects, specific to the job)
 *
 * And slotResult is usually at least:
 *
 * \code
 *  if ( !task->succeeded() )
 *      Kopete::UI::Global::showTaskError( task );
 * \endcode
 *
 * Much of the ideas (and some of the documentation and function names) for this
 * class come from KIO::Job.
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class Task : public QObject
{
	Q_OBJECT

protected:
	Task();
public:
	~Task();

	/**
	 * Returns whether the task completed successfully.
	 * Only call this method from the slot connected to result().
	 * @return if the task succeeded, returns true, otherwise returns false.
	 */
	bool succeeded() const;
	/**
	 * Converts an error code and a non-i18n error message into an
	 * error message in the current language. The low level (non-i18n)
	 * error message (usually a url) is put into the translated error
	 * message using %%1.
	 *
	 * Use this to display the error yourself, but for a dialog box
	 * use Kopete::UI::Global::showTaskError. Do not call it if succeeded()
	 * returns true.
	 * @return the error message and if there is no error, a message
	 *         telling the user that the app is broken, so check with
	 *         succeeded() whether there is an error.
	 */
	const QString &errorString() const;

	/** Flags for the abort() function */
	enum AbortFlags { AbortNormal = 0, AbortEmitResult = 1 };
public slots:
	/**
	 * Abort this task.
	 * This aborts all subtasks and deletes the task.
	 *
	 * @param flags a combination of flags from AbortFlags. If AbortEmitResult is
	 *        set, Job will emit the result signal. AbortEmitResult is removed
	 *        from the flags passed to the abort function of subtasks.
	 */
	virtual void abort( int flags = AbortNormal );

signals:
	/**
	 * Emitted when the task is finished, in any case (completed, canceled,
	 * failed...). Use error() to find the result.
	 * @param task the task that emitted this signal
	 */
	void result( Kopete::Task *task );
	/**
	 * Emitted to display status information about this task.
	 * Examples of messages are:
	 *   "Removing ICQ contact Joe from server-side list",
	 *   "Loading account plugin", etc.
	 * @param task the task that emitted this signal
	 * @param message the info message
	 */
	void statusMessage( Kopete::Task *task, const QString &message );

protected:
	/**
	 * Add a task that has to be completed before a result is emitted. This
	 * obviously should not be called after the finish signal is emitted by
	 * the subtask.
	 *
	 * @param task the subtask to add
	 */
	virtual void addSubtask( Task *task );

	enum RemoveSubtaskIfLast { IfLastDoNothing, IfLastEmitResult };
	/**
	 * Mark a sub job as being done. If it's the last to
	 * wait on the job will emit a result - jobs with
	 * two steps might want to override slotResult
	 * in order to avoid calling this method.
	 *
	 * @param task the subjob to add
	 * @param actionIfLast the action to take if this is the last subtask.
	 *        If set to IfLastEmitResult, the error information from @p task
	 *        will be copied to this object, and emitResult() will be called.
	 */
	virtual void removeSubtask( Task *task, RemoveSubtaskIfLast actionIfLast = IfLastEmitResult );

	enum Result { ResultFailed = 0, ResultSucceeded = 1 };
	/**
	 * Utility function to emit the result signal, and suicide this job.
	 * Sets the stored result and error message to @p result and @p errorMessage.
	 * You should call this instead of emitting the result() signal yourself.
	 */
	void emitResult( Result result = ResultSucceeded, const QString &errorMessage = QString::null );

protected slots:
	/**
	 * Called whenever a subtask finishes.
	 * The default implementation checks for errors and propagates
	 * them to this task, then calls removeSubtask().
	 * Override if you want to provide a different @p actionIfLast to
	 * removeSubtask, or want to perform some other processing in response
	 * to a subtask finishing
	 * @param task the subtask that finished
	 * @see result()
	 */
	virtual void slotResult( Kopete::Task *task );

private:
	class Private;
	Private *d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:
