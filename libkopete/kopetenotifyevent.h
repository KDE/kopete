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

class QDomElement;

class KopeteNotifyEvent
{
public:
	KopeteNotifyEvent( const bool suppressCommon = true );
	~KopeteNotifyEvent();

	bool suppressCommon() const;
	KopeteEventPresentation *presentation( const KopeteEventPresentation::PresentationType type ) const;
	void setPresentation( const KopeteEventPresentation::PresentationType type, KopeteEventPresentation * );
	void removePresentation( const KopeteEventPresentation::PresentationType type );
	/**
	 * @return true if the presentation was single shot 
	 */
	bool firePresentation( const KopeteEventPresentation::PresentationType type );
	
	void setSuppressCommon( bool suppress );
	const QValueList<QDomElement> toXML() const;
	QString toString();
private:
	QString m_event;
	KopeteEventPresentation *m_sound;
	KopeteEventPresentation *m_message;
	KopeteEventPresentation *m_chat;
	bool m_suppressCommon;
};

#endif
