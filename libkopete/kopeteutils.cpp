/*
    Kopete Utils.
    Copyright (c) 2005 Duncan Mac-Vicar Prett <duncan@kde.org>

      isHostReachable function code derived from KDE's HTTP kioslave
        Copyright (c) 2005 Waldo Bastian <bastian@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteutils.h"

#include <qmap.h>
#include <QPixmap>
#include <QByteArray>

#include <kmessagebox.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include "kopeteaccount.h"
#include "knotification.h"
#include "kopeteutils_private.h"
#include "kopeteuiglobal.h"
#include "kopetestatusmanager.h"

namespace Kopete
{
namespace Utils
{

struct DefaultStrings
{
    DefaultStrings() :
        notifyConnectionLost_DefaultMessage(i18n("You have been disconnected.")),
        notifyConnectionLost_DefaultCaption(i18n("Connection Lost.")),
        notifyConnectionLost_DefaultExplanation(i18n("Kopete lost the channel used to talk to the instant messaging system.\nThis can be because either your internet access went down, the service is experiencing problems, or the service disconnected you because you tried to connect with the same account from another location. Try connecting again later.")),

        notifyCannotConnect_DefaultMessage(i18n("Cannot connect with the instant messaging server or peers.")),
        notifyCannotConnect_DefaultCaption(i18n("Cannot connect.")),
        notifyCannotConnect_DefaultExplanation(i18n("This means Kopete cannot reach the instant messaging server or peers.\nThis can be because either your internet access is down or the server is experiencing problems. Try connecting again later."))
    {
    }
    const QString notifyConnectionLost_DefaultMessage;
    const QString notifyConnectionLost_DefaultCaption;
    const QString notifyConnectionLost_DefaultExplanation;

    const QString notifyCannotConnect_DefaultMessage;
    const QString notifyCannotConnect_DefaultCaption;
    const QString notifyCannotConnect_DefaultExplanation;
};
K_GLOBAL_STATIC(DefaultStrings, defaultStrings)

void notify( QPixmap pic, const QString &eventid, const QString &caption, const QString &message, const QString explanation, const QString debugInfo)
{
	Q_UNUSED(caption);

	if ( Kopete::StatusManager::self()->globalStatusCategory() == Kopete::OnlineStatusManager::Busy )
		return;

	QStringList actions;
		if ( !explanation.isEmpty() )
			actions  << i18n( "More Information..." );
		kDebug( 14010 ) ;
		KNotification *n = new KNotification( eventid , 0l );
		n->setActions( actions );
		n->setText( message );
		n->setPixmap( pic );
		ErrorNotificationInfo info;
		info.explanation = explanation;
		info.debugInfo = debugInfo;

		NotifyHelper::self()->registerNotification(n, info);
		QObject::connect( n, SIGNAL(activated(uint)) , NotifyHelper::self() , SLOT(slotEventActivated(uint)) );
		QObject::connect( n, SIGNAL(closed()) , NotifyHelper::self() , SLOT(slotEventClosed()) );
		
		n->sendEvent();
}

void notifyConnectionLost( const Account *account, const QString caption, const QString message, const QString explanation, const QString debugInfo)
{
	if (!account)
		return;

	notify( account->accountIcon(KIconLoader::SizeMedium), QString::fromLatin1("connection_lost"), caption.isEmpty() ? defaultStrings->notifyConnectionLost_DefaultCaption : caption, message.isEmpty() ? defaultStrings->notifyConnectionLost_DefaultMessage : message, explanation.isEmpty() ? defaultStrings->notifyConnectionLost_DefaultExplanation : explanation, debugInfo);
}

void notifyCannotConnect( const Account *account, const QString explanation, const QString debugInfo)
{
	Q_UNUSED(explanation);

	if (!account)
		return;

	notify( account->accountIcon(KIconLoader::SizeMedium), QString::fromLatin1("cannot_connect"), defaultStrings->notifyCannotConnect_DefaultCaption, defaultStrings->notifyCannotConnect_DefaultMessage, defaultStrings->notifyCannotConnect_DefaultExplanation, debugInfo);
}

} // end ns ErrorNotifier
} // end ns Kopete

