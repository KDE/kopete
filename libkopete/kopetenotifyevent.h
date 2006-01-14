/*
    kopetenotifyevent.h - Container for presentations of an event

    Copyright (c) 2004 by Will Stephenson     <lists@stevello.free-online.co.uk>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETENOTIFYEVENT_H
#define KOPETENOTIFYEVENT_H

#include <qstring.h>
#include <qvaluelist.h>
#include "kopeteeventpresentation.h"

#include "kopete_export.h"

class QDomElement;

namespace Kopete
{

class KOPETE_EXPORT NotifyEvent
{
public:
	NotifyEvent( const bool suppressCommon = false );
	~NotifyEvent();

	bool suppressCommon() const;
	EventPresentation *presentation( const EventPresentation::PresentationType type ) const;
	void setPresentation( const EventPresentation::PresentationType type, EventPresentation * );
	void removePresentation( const EventPresentation::PresentationType type );
	/**
	 * @return true if the presentation was single shot 
	 */
	bool firePresentation( const EventPresentation::PresentationType type );
	
	void setSuppressCommon( bool suppress );
	const QValueList<QDomElement> toXML() const;
	QString toString();
private:
	QString m_event;
	EventPresentation *m_sound;
	EventPresentation *m_message;
	EventPresentation *m_chat;
	bool m_suppressCommon;
};

}

#endif
