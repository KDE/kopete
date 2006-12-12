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

#include <qmap.h>

#include <kmessagebox.h>
#include <knotifyclient.h>

#include <kapplication.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kdatastream.h>
#include <kdebug.h>
#include <kiconloader.h>

#include "kopeteaccount.h"
#include "knotification.h"
#include "kopeteutils_private.h"
#include "kopeteutils.h"
#include "kopeteuiglobal.h"

static const QString notifyConnectionLost_DefaultMessage = i18n("You have been disconnected.");
static const QString notifyConnectionLost_DefaultCaption = i18n("Connection Lost.");
static const QString notifyConnectionLost_DefaultExplanation = i18n("Kopete lost the channel used to talk to the instant messaging system.\nThis can be because either your internet access went down, the service is experiencing problems, or the service disconnected you because you tried to connect with the same account from another location. Try connecting again later.");

static const QString notifyCannotConnect_DefaultMessage = i18n("Can't connect with the instant messaging server or peers.");
static const QString notifyCannotConnect_DefaultCaption = i18n("Can't connect.");
static const QString notifyCannotConnect_DefaultExplanation = i18n("This means Kopete can't reach the instant messaging server or peers.\nThis can be because either your internet access is down or the server is experiencing problems. Try connecting again later.");

namespace Kopete
{
namespace Utils
{

void notify( QPixmap pic, const QString &eventid, const QString &caption, const QString &message, const QString explanation, const QString debugInfo)
{
		QString action;
		if ( !explanation.isEmpty() )
			action = i18n( "More Information..." );
		kdDebug( 14010 ) << k_funcinfo <<  endl;
		KNotification *n = KNotification::event( eventid, message, pic , 0L , action );
		ErrorNotificationInfo info;
		info.explanation = explanation;
		info.debugInfo = debugInfo;

		NotifyHelper::self()->registerNotification(n, info);
		QObject::connect( n, SIGNAL(activated(unsigned int )) , NotifyHelper::self() , SLOT( slotEventActivated(unsigned int) ) );
		QObject::connect( n, SIGNAL(closed()) , NotifyHelper::self() , SLOT( slotEventClosed() ) );
}

void notifyConnectionLost( const Account *account, const QString &caption, const QString &message, const QString &explanation, const QString &debugInfo )
{
	if (!account)
		return;

	notify( account->accountIcon(32), QString::fromLatin1("connection_lost"), caption.isEmpty() ? notifyConnectionLost_DefaultCaption : caption, message.isEmpty() ? notifyConnectionLost_DefaultMessage : message, explanation.isEmpty() ? notifyConnectionLost_DefaultExplanation : explanation, debugInfo);
}

bool isHostReachable(const QString &host)
{
	const int NetWorkStatusUnknown = 1;
	const int NetWorkStatusOnline = 8;
	QCString replyType;
	QByteArray params;
	QByteArray reply;

	QDataStream stream(params, IO_WriteOnly);
	stream << host;

	if ( KApplication::kApplication()->dcopClient()->call( "kded", "networkstatus", "status(QString)", params, replyType, reply ) && (replyType == "int") )
	{
		int result;
		QDataStream stream2( reply, IO_ReadOnly );
		stream2 >> result;
		return (result != NetWorkStatusUnknown) && (result != NetWorkStatusOnline);
	}
	return false; // On error, assume we are online
}

void notifyCannotConnect( const Account *account, const QString &explanation, const QString &debugInfo)
{
	if (!account)
		return;

	notify( account->accountIcon(), QString::fromLatin1("cannot_connect"), notifyCannotConnect_DefaultCaption, notifyCannotConnect_DefaultMessage, notifyCannotConnect_DefaultExplanation, debugInfo);
}

void notifyConnectionError( const Account *account, const QString &caption, const QString &message, const QString &explanation, const QString &debugInfo )
{
	if (!account)
		return;

	// TODO: Display a specific default connection error message, I don't want to introducte too many new strings
	notify( account->accountIcon(32), QString::fromLatin1("connection_error"), caption, message, explanation, debugInfo);
}

void notifyServerError( const Account *account, const QString &caption, const QString &message, const QString &explanation, const QString &debugInfo )
{
	if (!account)
		return;

	// TODO: Display a specific default server error message, I don't want to introducte too many new strings
	notify( account->accountIcon(32), QString::fromLatin1("server_error"), caption, message, explanation, debugInfo);
}

} // end ns ErrorNotifier
} // end ns Kopete

