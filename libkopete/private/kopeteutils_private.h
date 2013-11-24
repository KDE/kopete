/*
    Kopete Utils.

    Copyright (c) 2005 Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETE_UTILS_PRIVATE_H
#define KOPETE_UTILS_PRIVATE_H

#include <qmap.h>
#include "qobject.h"
#include "qstring.h"
#include "qpixmap.h"

class KNotification;

namespace Kopete
{

namespace Utils
{

typedef struct
{
	QString caption;
	QString explanation;
	QString debugInfo;
} ErrorNotificationInfo;

class NotifyHelper : public QObject
{
Q_OBJECT
public:
	static NotifyHelper* self();
	void registerNotification(const KNotification* event, ErrorNotificationInfo error);
	void unregisterNotification(const KNotification* event);
public slots:
	void slotEventActivated(unsigned int action);
	void slotEventClosed();
private:
	NotifyHelper();
	~NotifyHelper();
	QMap<const KNotification*, ErrorNotificationInfo> m_events;
	static NotifyHelper *s_self;
};

} // end ns Utils
} // end ns Kopete

#endif
