/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <kdebug.h>
#include <TelepathyQt4/PendingOperation>

//#define SHOW_MESSAGEBOX_ERRORS

class TelepathyCommons
{
public:
    static bool isOperationError(Tp::PendingOperation* operation) {
        if (operation->isError()) {
            kDebug(TELEPATHY_DEBUG_AREA) << operation->errorName() << operation->errorMessage();
#ifdef SHOW_MESSAGEBOX_ERRORS
            KMessageBox::information(0, i18n("Error: %1\n%2", operation->errorName() , operation->errorMessage()));
#endif
            return true;
        }

        return false;
    }
};

#endif //COMMON_H_

