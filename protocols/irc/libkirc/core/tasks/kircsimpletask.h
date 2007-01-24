/*
    kircsimpletask.h - IRC Simple Task

    Copyright (c) 2004-2006 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2004-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCSIMPLETASK_H
#define KIRCSIMPLETASK_H

#include "kirctask.h"

namespace KIrc
{

/**
 * This task allow an implementor to add task in a simple way.
 * Following the patterns for the method names, will automagically call the correct methods (Thanks to the moc tool).
 */
class KIRC_EXPORT SimpleTask
	: public KIrc::Task
{
	Q_OBJECT
	Q_ENUMS(Status)

public:
	SimpleTask(QObject *parent = 0);
	~SimpleTask();

public:
	/**
	 * Tries to handle a user command.
	 *
	 * @return the status of the command handling.
	 */
	virtual Status doCommand(KIrc::Message command);

	/**
	 * Tries to handle an event.
	 *
	 * @return the status of the event handling.
	 */
//	virtual Status doEvent(KIrc::Event event);

	/**
	 * Tries to handle a server message.
	 *
	 * @return the status of the message handling.
	 */
	virtual Status doMessage(KIrc::Message message);

/*
	virtual QStringList listCommands();

	virtual QString getCommandHelp(const QString &command);
*/

signals:
	void postEvent(KIrc::Event event);

	void postCommand(KIrc::Message command);

	void postMessage(KIrc::Message message);

protected:
	Status doMessage(const char *commandPrefix, KIrc::Message message);

private:
	Q_DISABLE_COPY(SimpleTask)

	class Private;
	Private * const d;
};

}

#endif
