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

#include <telepathyaddedinfoevent.h>

#include <telepathyprotocolinternal.h>

#include <KDebug>

TelepathyAddedInfoEvent::TelepathyAddedInfoEvent(Tp::ContactPtr contact, Kopete::Account *account)
 : Kopete::AddedInfoEvent(contact->id(), account),
   m_contact(contact)
{
    kDebug();
}

TelepathyAddedInfoEvent::~TelepathyAddedInfoEvent()
{
    kDebug();
}

Tp::ContactPtr TelepathyAddedInfoEvent::contact() const
{
    return m_contact;
}


#include "telepathyaddedinfoevent.moc"

