// oscarlistnonservercontacts.h
// Copyright (C)  2005  Matt Rogers <mattr@kde.org>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#ifndef OSCARLISTNONSERVERCONTACTS_H
#define OSCARLISTNONSERVERCONTACTS_H

#include <kdialogbase.h>
#include "kopete_export.h"

class OscarListContactsBase;
class QStringList;

class KOPETE_EXPORT OscarListNonServerContacts : public KDialogBase
{
Q_OBJECT
public:
    OscarListNonServerContacts( QWidget* parent );
    ~OscarListNonServerContacts();

    void addContacts( const QStringList& contactList );
    QStringList nonServerContactList() const;

    bool onlyShowOnce();

protected:
    virtual void slotOk();
    virtual void slotCancel();

signals:
    void closing();

private:
    OscarListContactsBase* m_contactsList;
    QStringList m_nonServerContacts;

};
#endif
