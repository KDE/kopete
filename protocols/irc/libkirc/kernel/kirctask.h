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

class Context;

class KIRC_EXPORT Task
	: public QObject
{
	Q_OBJECT

public:
	explicit Task(QObject *parent = 0);
	virtual ~Task();

public:
	/**
	 * Tries to handle an event.
	 *
	 * @return the status of the event handling.
	 */
	virtual void ircEvent(KIrc::Context *context, KIrc::Event *event);

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
