/*
    kopetemessageevent.h - Kopete Message Event

    Copyright (c) 2003 by Olivier Goffart <ogoffart@tiscalinet.be>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002 by Hendrik vom Lehn <hvl@linux-4-ever.de>
    Copyright (c) 2004 by Richard Smith <richard@metafoo.co.uk>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEMESSAGEEVENT_H
#define KOPETEMESSAGEEVENT_H

#include <qobject.h>

#include "kopetemessage.h"

namespace Kopete
{

/**
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 *
 * Kopete::MessageEvent is used when a new messages arrives, it is
 * caught by the UI. It contains just informations about
 * the message, and a signal when it is terminated (i.e.
 * the message is read
 **/
class MessageEvent : public QObject
{
	Q_OBJECT

public:
	MessageEvent(const Kopete::Message& , QObject* parent=0L, const char *name=0L);
	~MessageEvent();

	/**
	 * @return the message
	 */
	Kopete::Message message();

	/**
	 * The state of the event.
	 * - Nothing means that the event has not been accepted or ignored
	 * - Applied if the event has been applied
	 * - Ignored if the event has been ignored
	 */
	enum EventState { Nothing , Applied , Ignored };

	EventState state();


signals:
	/**
	 * The event is processed
	 **/
	void done(Kopete::MessageEvent *);

private:
	Kopete::Message m_message;
	EventState m_state;

public slots:
	/**
	 * execute the event
	 */
	void apply();
	/**
	 * ignore the event
	 */
	void ignore();

};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

