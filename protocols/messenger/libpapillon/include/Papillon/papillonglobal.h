/*
   papillonglobal.h - Global utility functions for libpapillon

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

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

#include <Papillon/Enums>
#include <Papillon/Macros>

namespace Papillon
{
namespace Global
{
/**
 * @file
 * @brief Global methods used inside libpapillon
 */

/**
 * @brief Convert a numeric presence to string.
 * Handy function to get the string represention of Papillon::OnlineStatus.
 * @param presence Presence enum value to convert to string
 */
QString PAPILLON_EXPORT presenceToString(Papillon::Presence::Status presence);

/**
 * @brief Convert a string presence to enum value
 * Handy function to get the enum value representation of a string.
 * @param presence Presence string to convert to enum value
 */
Papillon::Presence::Status PAPILLON_EXPORT stringToPresence(const QString &presence);

} // Global
} // Papillon

#endif
