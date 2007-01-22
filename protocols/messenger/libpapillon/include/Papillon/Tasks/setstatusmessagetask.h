/*
   setstatusmessagetask.h - Set status message on server.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#ifndef PAPILLONSETSTATUSMESSAGETASK_H
#define PAPILLONSETSTATUSMESSAGETASK_H

#include <Papillon/Macros>
#include <Papillon/Task>

namespace Papillon
{

class StatusMessage;
/**
 * @class SetStatusMessageTask setstatusmessagetask.h <Papillon/Tasks/SetStatusMessageTask>
 * @brief Set personal status message on server.
 *
 * You should listen to finished() signal to confirm that the status message was set on server.
 *
 * @code
 * SetStatusMessageTask *setStatus = new SetStatusMessageTask( connection()->rootTask() );
 * setStatus->setStatusMessage( statusMessage );
 * setStatus->go(Task::AutoDelete);
 * @endcode
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT SetStatusMessageTask : public Papillon::Task
{
	Q_OBJECT
public:
	/**
	 * @brief Create a new SetStatusMessageTask
	 * @param parent Root Task.
	 */
	explicit SetStatusMessageTask(Papillon::Task *parent);
	/**
	 * d-tor
	 */
	~SetStatusMessageTask();

	/**
	 * @brief Set the StatusMessage to be send on server.
	 * @param statusMessage StatusMessage to be send on server.
	 */
	void setStatusMessage(const Papillon::StatusMessage &statusMessage);
	
	/**
	 * @brief Check if the given Transfer has the confirmation of the command.
	 * @param transfer Transfer to check.
	 * @return true if we accept this transfer.
	 */
	virtual bool take(Papillon::Transfer *transfer);

protected:
	/**
	 * @brief Send the set status message command on server.
	 */
	virtual void onGo();

private:
	class Private;
	Private *d;
};

}
#endif
