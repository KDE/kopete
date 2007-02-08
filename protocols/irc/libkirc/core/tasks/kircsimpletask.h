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
	explicit SimpleTask(QObject *parent = 0);
	~SimpleTask();

public:
	/**
	 * Tries to handle an event.
	 *
	 * @return the status of the event handling.
	 */
	virtual Status doEvent(KIrc::Event *event);
/*
	virtual QStringList listCommands();

	virtual QString getCommandHelp(const QString &command);
*/

private:
	Q_DISABLE_COPY(SimpleTask)

	class Private;
	Private * const d;
};

}

#endif
