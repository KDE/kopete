/*
    kopeteevent.h - Kopete Event

    Copyright (c) 2003 by Olivier Goffart <ogoffart@tiscalinet.be>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002 by Hendrik vom Lehn <hvl@linux-4-ever.de>

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

#ifndef KOPETEEVENT_H
#define KOPETEEVENT_H

#include <qobject.h>

#include "kopetemessage.h"

/**
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 *
 * KopeteEvent is used when a new messages arrives, it is
 * caught by the UI. It contains just informations about
 * the message, and a signal when it is terminated (i.e.
 * the message is read
 **/
class KopeteEvent : public QObject
{
	Q_OBJECT

public:
	KopeteEvent(const Kopete::Message& , QObject* parent=0L, const char *name=0L);
	~KopeteEvent();

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
	void done(KopeteEvent *);

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

#endif

// vim: set noet ts=4 sts=4 sw=4:

