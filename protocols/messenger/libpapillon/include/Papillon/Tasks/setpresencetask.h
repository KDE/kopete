/*
   setpresencetask.h - Set our own presence on Windows Live Messenger service.

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
#ifndef PAPILLONSETPRESENCETASK_H
#define PAPILLONSETPRESENCETASK_H

#include <Papillon/Macros>
#include <Papillon/Enums>
#include <Papillon/Task>

namespace Papillon
{

/**
 * @class SetPresenceTask setpresencetask.h <Papillon/Tasks/SetPresenceTask>
 * @brief Change our own presence on Windows Live Messenger.
 *
 * @code
 * Papillon::SetPresenceTask *presenceTask = new Papillon::SetPresenceTask(connection->rootTask());
 * presenceTask->setPresence( Papillon::Presence::Online );
 * presenceTask->setClientFeatures(...); // Optional
 * presenceTask->setMsnObject( msnObject.toString() ); // Optional
 * connect(presenceTask, SIGNAL(finished(Papillon::Task*)), this, SLOT(slotPresenceSent(Papillon::Task*)));
 *
 * presenceTask->go(true);
 * @endcode
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT SetPresenceTask : public Papillon::Task
{
	Q_OBJECT
public:
	/**
	 * @brief Create a task to set the presence on server.
	 * @param parent Root task.
	 */
	explicit SetPresenceTask(Papillon::Task *parent);
	/**
	 * d-tor
	 */
	~SetPresenceTask();

	/**
	 * @brief Give the online status to be set on server.
	 * @param onlineStatus online status to set.
	 */
	void setPresence( Papillon::Presence::Status onlineStatus );

	/**
	 * @brief Set our features.
	 * @param features Features to set.
	 */
	void setClientFeatures( Papillon::ClientInfo::Features features );
	//TODO: void setMsnObject();

	/**
	 * @brief Check if the transfer is for us.
	 * In this case, emit finished() signal when we receive a acknowledge of the command.
	 * @param transfer Transfer to check.
	 */
	virtual bool take(Papillon::Transfer *transfer);

protected:
	/**
	 * @brief Send the set presence command to server.
	 */
	virtual void onGo();

private:
	class Private;
	Private *d;
};

}
#endif
