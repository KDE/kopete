/*
    kopetenotifydataobject.h - Kopete Custom Notify Data Object

    Copyright (c) 2004 by Will Stephenson       <lists@stevello.free-online.co.uk>

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
#ifndef KOPETENOTIFYDATAOBJECT_H
#define KOPETENOTIFYDATAOBJECT_H

#include <qdict.h>
#include <qstring.h>
#include <qvaluelist.h>

class QDomElement;
class KopeteNotifyEvent;

/**
 * Contains custom notification control and storage functionality
 */

class KopeteNotifyDataObject
{
	public:
		KopeteNotifyDataObject();
		~KopeteNotifyDataObject();
		// Notify events
		KopeteNotifyEvent *notifyEvent( const QString &event ) const;
		void setNotifyEvent( const QString &event, 
				KopeteNotifyEvent *notifyEvent );
		bool removeNotifyEvent( const QString &event );
		// Serialization
	protected:
		QDomElement notifyDataToXML();
		bool notifyDataFromXML( const QDomElement& element );
	private:
		QDict<KopeteNotifyEvent> m_events;
};

#endif
