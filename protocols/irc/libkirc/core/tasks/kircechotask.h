/*
    kircechotask.h - IRC Echo task

    Copyright (c) 2006      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2006      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCECHOTASK_H
#define KIRCECHOTASK_H

#include "kirctask.h"

namespace KIrc
{

class Event;

/**
 * A simple task that echo's every messages, to the event handling system.
 *
 * Thougth it send events it never consider to have done anything and thus allways return 
 * Task::NotHandled when calling Task::doMessage. 
 *
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 */
class KIRC_EXPORT EchoTask
	: public KIrc::Task
{
	Q_OBJECT

public:
#warning Make singleton
	EchoTask(QObject *parent = 0);
	~EchoTask();

public:
	virtual Status doMessage(KIrc::Message msg);

private:
	Q_DISABLE_COPY(EchoTask)

	class Private;
	Private * const d;
};

}

#endif

