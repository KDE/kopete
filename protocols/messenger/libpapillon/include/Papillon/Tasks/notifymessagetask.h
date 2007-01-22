/*
   notifymessagetask.h - Notify about new messages on Notification server.

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
#ifndef PAPILLONNOTIFYMESSAGETASK_H
#define PAPILLONNOTIFYMESSAGETASK_H

#include <Papillon/Task>
#include <Papillon/Macros>

namespace Papillon
{

class MimeHeader;
/**
 * @class NotifyMessageTask notifymessagetask.h <Papillon/Tasks/NotifyMessageTask>
 * @brief Notify about new messages on Notification server.
 *
 * NotifyMessageTask read the MSG command on the notification server connection.
 * Examples of received messages:
 * -Initial profile message
 * -Initial new email notification (for Hotmail)
 * -Email and Hotmail notifications.
 *
 * @author Michaël Larouche <larouche@kde.org>
*/
class PAPILLON_EXPORT NotifyMessageTask : public Papillon::Task
{
	Q_OBJECT
public:
	explicit NotifyMessageTask(Papillon::Task *parent);
	virtual ~NotifyMessageTask();

	virtual bool take(Papillon::Transfer *transfer);
	
signals:
	/**
	 * Emitted when it receive initial profile message.
	 * @param profileMessage Initial profile message as MimeHeader
	 */
	void profileMessage(const Papillon::MimeHeader &profileMessage);

private:
	virtual bool forMe(Papillon::Transfer *transfer) const;

	class Private;
	Private *d;
};

}

#endif
