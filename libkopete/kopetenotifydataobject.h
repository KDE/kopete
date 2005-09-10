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

#include "kopete_export.h"

class QDomElement;

namespace Kopete
{

class NotifyEvent;

/**
 * Contains custom notification control and storage functionality
 */

class KOPETE_EXPORT NotifyDataObject
{
	public:
		NotifyDataObject();
		~NotifyDataObject();
		// Notify events
		NotifyEvent *notifyEvent( const QString &event ) const;
		void setNotifyEvent( const QString &event, 
				NotifyEvent *notifyEvent );
		bool removeNotifyEvent( const QString &event );
		// Serialization
	protected:
		QDomElement notifyDataToXML();
		bool notifyDataFromXML( const QDomElement& element );
	private:
		class Private;
		NotifyDataObject::Private* d;
};

}

#endif
