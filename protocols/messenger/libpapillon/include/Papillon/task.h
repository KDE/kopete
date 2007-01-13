/*
   task.h - Papillon Task base class.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Based on code Copyright (c) 2004 Matt Rogers <mattr@kde.org>
   Based on code copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
   Based on Iris, Copyright (C) 2003 Justin Karneges

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#ifndef PAPILLON_TASK_H
#define PAPILLON_TASK_H

#include <QtCore/QObject>
#include <QtCore/QFlags>
#include <Papillon/Macros>

class QString;

namespace Papillon
{

class Transfer;
class Connection;
/**
 * @class Task task.h <Papillon/Task>
 * @brief Task is the base class for all Task accros Papillon.
 *
 * Derived class must implement these following methods: <br>
 * take() - Used to listen and proceed incoming Transfer. <br>
 * onGo() - Called by go(), start the task. <br>
 * 
 * Devired class may implement these following methods: <br>
 * forMe() - Used to check if the incoming Transfer is for us. <br>
 * onDisconnect() - If you need to do something special when a Connection disconnect. <br>
 *
 * @author Michaël Larouche <larouche@kde.org>
 * @author Matt Rogers  <mattr@kde.org>
 * @author SuSE Linux AG <http://www.suse.com>
 * @author Justin Karneges
 */
class PAPILLON_EXPORT Task : public QObject
{
	Q_OBJECT
public:
	/**
	 * Enum for status code
	 */
	enum StatusCode
	{
		/**
		 * Task failed because it was disconnected.
		 */
		ErrorDisconnected
	};
	
	/**
	 * @brief This enum is the parameters that the go() method accepts.
	 */
	enum GoParameter
	{
		GoNothing, ///<Do nothing
		AutoDelete ///<Auto delete the task after completion.
	};
	Q_DECLARE_FLAGS(GoParameters, GoParameter)

	/**
	 * Create a new task.
	 * @param parent root Task for this Task
	 */
	Task(Task *parent);
	/**
	 * Create a root task
	 * @param connection Connection pointer for this task
	 * @param isRoot if the Task is a root one or not.
	 */
	Task(Connection *connection, bool isRoot);
	/**
	 * Task d-tor.
	 */
	virtual ~Task();

	/**
	 * Return the parent Task.
	 * @return the parent Task.
	 */
	Task *parent() const;
	/**
	 * Return a reference to the connection used by this task.
	 * @return Connection pointer.
	 */
	Connection* connection() const;

	/**
	 * Get the current transfer if any.
	 * Need to be set with setTransfer().
	 * @return the current Transfer or 0 if this Task do not store any Transfer.
	 * TODO: Remove ?
	 */
	Transfer *transfer() const;
	/**
	 * Direct setter for Tasks which don't have any fields
	 * @param transfer Transfer to set.
	 * TODO: Remove ?
	 */
	void setTransfer(Transfer *transfer);

	/**
	 * Call this the resulting slot of finished() signal.
	 * Check if the Task was successful or not.
	 * @return true if the Task was a success.
	 */
	bool success() const;
	/**
	 * Return the statusCode for this Task.
	 * @return the status code.
	 */
	int statusCode() const;
	/**
	 * Get a human readable string of the current status of the Task.
	 * @return status description.
	 */
	const QString &statusString() const;

	/**
	 * Start the Task.
	 * @param args See GoParameters for details
	 */
	void go(GoParameters args = GoNothing);

	/**
	 * Allows a task to examine an incoming Transfer and decide whether to 'take' it
	 * for further processing.
	 */
	virtual bool take(Transfer *transfer);
	/**
	 * Delete safetely this Task.
	 */
	void safeDelete();

signals:
	/**
	 * Emmited when the Task has finished.
	 * Check if the task was successful with success().
	 *
	 * @param task Pointer to the current Task.
	 */
	void finished(Papillon::Task *task);

protected:
	/**
	 * Derived classes should implement this method.
	 * Implements "start" of the Task.
	 */
	virtual void onGo();
	/**
	 * Derived classes should implement this method.
	 * Called when a disconnection occurs.
	 */
	virtual void onDisconnect();

	/**
	 * Helper method for derived Task to send a Transfer through the Connection.
	 */
	void send(Transfer *request);
	/**
	 * Helper method to terminate this Task successfully.
	 * @param code status code.
	 * @param str status string
	 */
	void setSuccess(int code=0, const QString &str = QLatin1String(""));
	/**
	 * Helper method to terminate this Task with an error.
	 * @param code status code.
	 * @param str status string.
	 */
	void setError(int code=0, const QString &str = QLatin1String(""));
	/**
	 * Debug helper.
	 * TODO: Remove ?
	 */
	void debug(const QString &);

	/**
	 * Used in take() to check if the offered transfer is for this Task
	 * @return true if this Task should take the Transfer.  Default impl always returns false.
	 */
	virtual bool forMe(Transfer *transfer) const;

private slots:
	/**
	 * Called when the Connection disconnect.
	 * Call onDisconnect().
	 */
	void connectionDisconnected();
	/**
	 * Finish this Task.
	 */
	void done();

private:
	/**
	 * @internal
	 * Init the private members.
	 */
	void init();

	class Private;
	Private *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( Task::GoParameters )

}
#endif
