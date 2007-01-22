/*
   notifystatusmessagetask.h - Notify contact status message changes.

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
#ifndef PAPILLONNOTIFYSTATUSMESSAGETASK_H
#define PAPILLONNOTIFYSTATUSMESSAGETASK_H

#include <Papillon/Macros>
#include <Papillon/Task>

namespace Papillon
{

class StatusMessage;
/**
 * @class NotifyStatusMessageTask notifystatusmessagetask.h <Papillon/Tasks/NotifyStatusMessageTask>
 * @brief Notify when a contact changes his status message.
 * This task watches for contact status message changes and 
 * emit ontactStatusMessageChanged() signal when it receive 
 * a status message update.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class NotifyStatusMessageTask : public Papillon::Task
{
	Q_OBJECT
public:
	/**
	 * @brief Create a new NotifyStatusMessageTask.
	 * @param parent Root Task.
	 */
	explicit NotifyStatusMessageTask(Papillon::Task *parent);
	/**
	 * d-tor
	 */
	~NotifyStatusMessageTask();

	/**
	 * @brief Watch in given Transfer for status message change from contacts.
	 * @param transfer Transfer to look for incoming status message change.
	 * @return true if we accept the Transfer.
	 */
	virtual bool take(Papillon::Transfer *transfer);

signals:
	/**
	 * Emitted when a contact has updated his status message.
	 * @param contactId Contact ID
	 * @param statusMessage Updated status message.
	 */
	void contactStatusMessageChanged(const QString &contactId, const Papillon::StatusMessage &statusMessage);

private:
	class Private;
	Private *d;
};

}

#endif
