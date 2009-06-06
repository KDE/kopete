/*
    kopetemessageevent.h - Kopete Message Event

    Copyright (c) 2003 by Olivier Goffart <ogoffart@kde.org>
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

#include <QtCore/QObject>

#include "kopetemessage.h"
#include "kopete_export.h"

namespace Kopete
{

/**
 * @author Olivier Goffart <ogoffart@kde.org>
 * @author Richard Smith   <richard@metafoo.co.uk>
 *
 * Kopete::MessageEvent is used when a new messages arrives, it is
 * caught by the UI. It contains just information about
 * the message, and a signal when it is terminated (i.e.
 * the message is read
 **/
class KOPETE_EXPORT MessageEvent : public QObject
{
	Q_OBJECT

public:
	explicit MessageEvent(const Kopete::Message& , QObject* parent=0L ); /* implicit */
	~MessageEvent();

	/**
	 * @return A copy of the message
	 */
	Kopete::Message message();
	
	/**
	 * Sets the message contained in this event.
	 * @param message The new value for the message
	 */
	void setMessage( const Kopete::Message &message );

	/**
	 * The state of the event.
	 * - @c Nothing means that the event has not been accepted or ignored
	 * - @c Applied if the event has been applied
	 * - @c Ignored if the event has been ignored
	 */
	enum EventState { Nothing , Applied , Ignored };

	EventState state();

public slots:
	/**
	 * @deprecated Use accept() instead to continue the processing of this event once the caller has moved to using MessageHandlers
	 * 
	 * execute the event
	 */
	void apply();
	
	/**
	 * @deprecated Use discard() instead to destroy this event once the caller has moved to using MessageHandlers
	 * 
	 * ignore the event
	 */
	void ignore();
	
	/**
	 * @brief Passes the event to the next handler
	 * 
	 * Call this when you've finished processing this event
	 */
	void accept();
	
	/**
	 * @brief Discards the event
	 *
	 * If this event should not be processed any further, this function
	 * should be called to discard it.
	 */
	void discard();
	
signals:
	/**
	 * The event has been processed
	 */
	void done(Kopete::MessageEvent *);
	
	/**
	 * The event has been discarded.
	 * @param event The event sending the signal.
	 */
	void discarded(Kopete::MessageEvent *event);
	
	/**
	 * The event has been accepted by its current handler.
	 * @param event The event sending the signal.
	 */
	void accepted(Kopete::MessageEvent *event);

private:
	class Private;
	Private * const d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

