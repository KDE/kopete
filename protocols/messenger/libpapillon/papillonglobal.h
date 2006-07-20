/*
   papillonglobal.h - Global utility functions for libpapillon

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONGLOBAL_H
#define PAPILLONGLOBAL_H

#include <QtCore/QString>
#include <QtCore/QLatin1String>

#include <papillon_enums.h>
#include <papillon_macros.h>

namespace Papillon
{

/**
 * @brief Convert a numeric presence status to string.
 * Handly function to get the string represention of Papillon::OnlineStatus.
 */
QString PAPILLON_EXPORT statusToString(Papillon::OnlineStatus::Status status);

Papillon::OnlineStatus::Status PAPILLON_EXPORT stringToStatus(const QString &status);

};
#endif
