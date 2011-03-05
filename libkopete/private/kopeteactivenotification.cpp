/*
    kopeteactivenotification.cpp - View Manager

    Kopete    (c) 2009 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteactivenotification.h"

#include <klocale.h>
#include <knotification.h>

namespace Kopete
{
        ActiveNotification::ActiveNotification( KNotification *notification, const QString& id_, ActiveNotifications& notifications_, const QString& title_, const QString& body_ )
          : QObject( notification ), id( id_ ),
            notifications( notifications_ ), nEventsSinceNotified( 0 ), title( title_ ), body( body_ )
        {
            notifications.insert( id, this );
            KNotification* aParent = static_cast<KNotification *>( parent() );
            if ( !title.isEmpty() )
            {
                aParent->setTitle( "<qt>" + title + "</qt>" );
            }
            aParent->setText( "<qt>" + body + "</qt>" );
        }

        /**
         * Remove active notification from queue
         */
        ActiveNotification::~ActiveNotification() {
            notifications.remove( id );
        }

        /**
         * Show this notification
         */
        void ActiveNotification::showNotification() {
            static_cast<KNotification *>( parent() )->sendEvent();
        }

        /**
         * received another message from a sender with a notification
         */
        void ActiveNotification::incrementMessages() {
            KLocalizedString append = ki18np( "+ %1 more message", "+ %1 more messages");
            KNotification *aParent = static_cast<KNotification *>( parent() );
            aParent->setText( "<qt>" + body + "<br/><small><font color=\"yellow\">" + append.subs( ++nEventsSinceNotified ).toString() + "</small></font></qt>" );
            aParent->sendEvent(); // Show it
        }
}
