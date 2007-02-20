/*
    kirctask.h - IRC Task

    Copyright (c) 2004-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2004-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCTASK_H
#define KIRCTASK_H

#include "kircevent.h"
#include "kircmessage.h"

#include <QtCore/QObject>

namespace KIrc
{

class KIRC_EXPORT Task
	: public QObject
{
	Q_OBJECT
	Q_ENUMS(Status)

public:
	enum Status
	{
		/**
		 * The command was unhandled by this task, and will try to be handled by next task.
		 */
		NotHandled = 0,

		/**
		 * The message doesn't require more plugin processing. 
		 */
		PluginHandled = 1 << 0,

		/**
		 * The message doesn't require more system precessing.
		 */
		SystemHandled = 1 << 1,

		/**
		 * The command was fully handled, and don't require more processing.
		 */
		FullyHandled = PluginHandled | SystemHandled 
	};

	explicit Task(QObject *parent = 0);
	~Task();

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
signals:
	void postEvent(KIrc::Event *event);

//	void postCommand(KIrc::Message command);

//	void postMessage(KIrc::Message message);

private:
	Q_DISABLE_COPY(Task)

	class Private;
	Private * const d;
};

}

#endif
