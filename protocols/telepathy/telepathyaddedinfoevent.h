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

#ifndef KOPETE_PROTOCOL_TELEPATHY_TELEPATHY_ADDED_INFO_EVENT_H
#define KOPETE_PROTOCOL_TELEPATHY_TELEPATHY_ADDED_INFO_EVENT_H

#include <kopeteaddedinfoevent.h>

#include <TelepathyQt4/Contact>

class TelepathyAddedInfoEvent : public Kopete::AddedInfoEvent
{
    Q_OBJECT

public:
    TelepathyAddedInfoEvent(Tp::ContactPtr contact, Kopete::Account *account);
    virtual ~TelepathyAddedInfoEvent();

    Tp::ContactPtr contact() const;

private:
    Tp::ContactPtr m_contact;
};


#endif  // Header guard

