/*
    kopetetask.h - Kopete Task

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

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
#include <kopete_export.h>
#include <kjob.h>

namespace Kopete
{

/**
 * The base class for all tasks.
 * For most tasks created in Kopete, the code looks like
 *
 * \code
 *   Kopete::Task *task = someobject->someoperation( some parameters );
 *   connect( task, SIGNAL( result( KJob* ) ),
 *            this, SLOT( slotResult( KJob* ) ) );
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
class KOPETE_EXPORT Task : public KJob
{
	Q_OBJECT

protected:
	Task();
public:
	virtual ~Task();

	/**
	 * Returns whether the task completed successfully.
	 * Only call this method from the slot connected to result().
	 * @return if the task succeeded, returns true, otherwise returns false.
	 */
	bool succeeded() const;

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

	enum Result { ResultFailed = 1, ResultSucceeded = 0 };
	/**
	 * Utility function to emit the result signal, and suicide this job.
	 * Sets the stored result and error message to @p result and @p errorMessage.
	 * You should call this instead of emitting the result() signal yourself.
	 */
	// NOTE: Named emitTaskResult to not conflit with KJob::emitResult
	//       and not cause a infinite loop (-DarkShock)
	void emitTaskResult( Result result = ResultSucceeded, const QString &errorMessage = QString::null );

	/**
	 * Utility function for inherited tasks.
	 * Set the result value. You should call emitResult instead.
	 * @param result the task result.
	 */
	void setResult( Result result );

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
	 * @note should probably be private
	 */
	virtual void slotResult( KJob *task );

private:
	class Private;
	Private *d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:
