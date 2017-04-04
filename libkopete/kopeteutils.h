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

#include <QObject>
#include <QString>
#include <QPixmap>

#include "libkopete_export.h"

namespace Kopete {
class Account;

namespace Utils {
void LIBKOPETE_EXPORT notify(QPixmap pic, const QString &eventid, const QString &caption, const QString &message, const QString explanation, const QString debugInfo);

/**
 * Notifies the user connection has been lost without coupling plugins with GUI code.
 *
 * @param account The account that lost the connection and wants to notify the user.
 * @param caption A brief subject line, used where possible if the presentation allows it.
 * @param message A short description of the error.
 * @param explanation A long description on how the error occurred and what the user can do about it.
 * @param debugInfo Debug info that can be sent to the developers or to the network service owners.
 *
 * You cannot provide debugInfo without an user explanation. If you don't provide a caption, message, or
 * explanation, Kopete will use a default explanation.
 */
void LIBKOPETE_EXPORT notifyConnectionLost(const Account *account, const QString caption = QString(), const QString message = QString(),
                                        const QString explanation = QString(), const QString debugInfo = QString());

/**
 * Notifies the user the server is not reachable without coupling plugins with GUI code.
 *
 * @param account The account that cannot establish a connection and want to notify the user about that.
 * @param explanation A long description on how the error occurred and what the user can do about it.
 * @param debugInfo Debug info that can be sent to the developers or to the network service owners.
 *
 * You cannot provide debugInfo without an user explanation. If you don't provide a caption, message, or
 * explanation, Kopete will use a default explanation.
 */
void LIBKOPETE_EXPORT notifyCannotConnect(const Account *account, const QString explanation = QString(), const QString debugInfo = QString());
} // end ns Utils
} // end ns Kopete

#endif
