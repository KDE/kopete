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

#include <task.h>
#include <papillon_macros.h>

namespace Papillon
{

class MimeHeader;
/**
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
	NotifyMessageTask(Task *parent);
	virtual ~NotifyMessageTask();

	virtual bool take(Transfer* transfer);
	
signals:
	/**
	 * Emitted when it receive initial profile message.
	 * @param authTicket Auth ticket.
	 * TODO: Use MimeHeader class for the signal
	 */
	void profileMessage(const Papillon::MimeHeader &profileMessage);

private:
	virtual bool forMe(Transfer *transfer) const;

	class Private;
	Private *d;
};

}

#endif
