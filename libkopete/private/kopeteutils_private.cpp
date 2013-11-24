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
#include "kopeteutils_private.h"

#include <qapplication.h>
#include <qmap.h>

#include <kmessagebox.h>

#include <kdebug.h>

#include "knotification.h"
#include "kopeteuiglobal.h"

namespace Kopete
{
namespace Utils
{

NotifyHelper* NotifyHelper::s_self = 0L;

NotifyHelper::NotifyHelper() : QObject(qApp)
{
}

NotifyHelper::~NotifyHelper()
{
	s_self = 0L;
}

NotifyHelper* NotifyHelper::self()
{
	if (!s_self)
		s_self = new NotifyHelper();

	return s_self;
}

void NotifyHelper::slotEventActivated(unsigned int /*action*/)
{
	const KNotification *n = dynamic_cast<const KNotification *>(QObject::sender());
	if (n)
	{
		ErrorNotificationInfo info = m_events[n];
		if ( info.debugInfo.isEmpty() )
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information, info.explanation, info.caption);
		else
			KMessageBox::queuedDetailedError( Kopete::UI::Global::mainWidget(), info.explanation, info.debugInfo, info.caption);

		unregisterNotification(n);
	}
}

void NotifyHelper::slotEventClosed()
{
	const KNotification *n = dynamic_cast<const KNotification *>(QObject::sender());
	if (n)
		unregisterNotification(n);
}

void NotifyHelper::registerNotification(const KNotification* event, ErrorNotificationInfo error)
{
	m_events.insert( event, error);
}

void NotifyHelper::unregisterNotification(const KNotification* event)
{
	m_events.remove(event);
}

} // end ns ErrorNotifier
} // end ns Kopete

#include "kopeteutils_private.moc"
