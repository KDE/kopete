/*
    Kopete Utils.

    Copyright (c) 2005 Duncan Mac-Vicar Prett <duncan@kde.org>

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

#ifndef KOPETE_UTILS_H
#define KOPETE_UTILS_H

#include "qobject.h"
#include "qstring.h"
#include "qpixmap.h"
#include "kopete_export.h"

class KNotification;

namespace Kopete
{

class Account;

namespace Utils
{

/**
 * Checks if host is accesible. Useful for plugins to check for disconnected events.
 *
 * @param host The host to be cheked
 */
bool isHostReachable( const QString &host );

/**
 * Notifies the user connection has been lost without coupling plugins with GUI code.
 *
 * @param account The account that lost the connection and wants to notify the user.
 * @param caption A brief subject line, used where possible if the presentation allows it.
 * @param message A short description of the error.
 * @param explanation A long description on how the error occured and what the user can do about it.
 * @param debugInfo Debug info that can be sent to the developers or to the network service owners.
 *
 * You can not provide debugInfo without an user explanation. If you don't provide a caption, message, or
 * explanation, Kopete will use a default explanation.
 */
void KOPETE_EXPORT notifyConnectionLost( const Account *account,
                                         const QString &caption = QString::null,
                                         const QString &message = QString::null,
                                         const QString &explanation = QString::null,
                                         const QString &debugInfo = QString::null );


/**
 * Notifies the user the server is not reachable without coupling plugins with GUI code.
 *
 * @param account The account that cannot establish a connection and want to notify the user about that.
 * @param explanation A long description on how the error occured and what the user can do about it.
 * @param debugInfo Debug info that can be sent to the developers or to the network service owners.
 *
 * You can not provide debugInfo without an user explanation. If you don't provide a caption, message, or
 * explanation, Kopete will use a default explanation.
 */
void KOPETE_EXPORT notifyCannotConnect( const Account *account,
                                        const QString &explanation = QString::null,
                                        const QString &debugInfo = QString::null);

/**
 * Notifies the user that an error on a connection occcured without coupling plugins with GUI code.
 *
 * @param account The account where the connection error occured and wants to notify the user.
 * @param caption A brief subject line, used where possible if the presentation allows it.
 * @param message A short description of the error.
 * @param explanation A long description on how the error occured and what the user can do about it.
 * @param debugInfo Debug info that can be sent to the developers or to the network service owners.
 *
 * You can not provide debugInfo without an user explanation. If you don't provide a caption, message, or
 * explanation, Kopete will use a default explanation.
 */
void KOPETE_EXPORT notifyConnectionError( const Account *account,
                                          const QString &caption = QString::null,
                                          const QString &message = QString::null,
                                          const QString &explanation = QString::null,
                                          const QString &debugInfo = QString::null );

/**
 * Notifies the user that an error on the server occcured without coupling plugins with GUI code.
 *
 * @param account The account where the server error occured and wants to notify the user.
 * @param caption A brief subject line, used where possible if the presentation allows it.
 * @param message A short description of the error.
 * @param explanation A long description on how the error occured and what the user can do about it.
 * @param debugInfo Debug info that can be sent to the developers or to the network service owners.
 *
 * You can not provide debugInfo without an user explanation. If you don't provide a caption, message, or
 * explanation, Kopete will use a default explanation.
 */
void KOPETE_EXPORT notifyServerError( const Account *account,
                                      const QString &caption = QString::null,
                                      const QString &message = QString::null,
                                      const QString &explanation = QString::null,
                                      const QString &debugInfo = QString::null );
} // end ns Utils
} // end ns Kopete

#endif
