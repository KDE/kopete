/*
   notifypresencetask.h - Notify about presence changes of contacts

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONNOTIFYPRESENCETASK_H
#define PAPILLONNOTIFYPRESENCETASK_H

#include <papillon_enums.h>
#include <papillon_macros.h>

#include <task.h>

namespace Papillon
{

/**
 * @brief Notify presence changes of contacts.
 * This task watches for presence change on Notification server and emit
 * contactStatusChanged() signal when it receive a presence change.
 *
 * @author Michaël Larouche
 */
class PAPILLON_EXPORT NotifyPresenceTask : public Papillon::Task
{
	Q_OBJECT
public:
	/**
	 * @brief Create a new NotifyPresenceTask
	 * @param parent Root task.
	 */
	NotifyPresenceTask(Papillon::Task *parent);
	/**
	 * d-tor
	 */
	~NotifyPresenceTask();

	/**
	 * @brief Check the given Transfer for presence change.
	 * @param transfer Given Transfer
	 * @return true if this task need to proceed this transfer.
	 */
	virtual bool take(Transfer *transfer);

signals:
	// TODO: Maybe add nickname, features and MsnObject to this signal
	/**
	 * @brief A contact status has changed.
	 * @param contactId the contact ID.
	 * @param status new online status of the contact.
	 */
	void contactStatusChanged(const QString &contactId, Papillon::OnlineStatus::Status status);

protected:
	/**
	 * @brief Check if the transfer is a presence change command.
	 * @param transfer Transfer to evaluate
	 * @return true if it's a presence change command.
	 */
	virtual bool forMe(Transfer *transfer) const;
	
private:
	class Private;
	Private *d;
};

}
#endif
