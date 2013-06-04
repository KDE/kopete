// oscarlistnonservercontacts.cpp

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


#include "oscarlistnonservercontacts.h"
#include "ui_oscarlistcontactsbase.h"
#include <qstringlist.h>
#include <qcheckbox.h>
#include <klocale.h>

OscarListNonServerContacts::OscarListNonServerContacts(QWidget* parent)
    : KDialog( parent )
{
    setCaption( i18n( "Add Contacts to Server List" ) );
    setButtons( KDialog::Yes | KDialog::Cancel | KDialog::No );

    QWidget* w = new QWidget( this );
    m_contactsList = new Ui::OscarListContactsBase;
    m_contactsList->setupUi( w );
    setMainWidget( w );

    setButtonText( Yes, i18n( "&Add" ) );
    setButtonText( Cancel, i18n( "Do &Not Add" ) );
    setButtonText( No, i18n( "&Delete" ) );
}

OscarListNonServerContacts::~OscarListNonServerContacts()
{
    delete m_contactsList;
}

void OscarListNonServerContacts::addContacts( const QStringList& contactList )
{
    m_nonServerContacts = contactList;
    m_contactsList->notServerContacts->addItems( contactList );
}

QStringList OscarListNonServerContacts::nonServerContactList() const
{
    return m_nonServerContacts;
}

bool OscarListNonServerContacts::onlyShowOnce()
{
    return m_contactsList->doNotShowAgain->isChecked();
}


void OscarListNonServerContacts::slotButtonClicked( int buttonCode )
{
	KDialog::slotButtonClicked(buttonCode);

	if( buttonCode == KDialog::Cancel ||
	    buttonCode == KDialog::Yes ||
	    buttonCode == KDialog::No )
    	emit closing();
}

#include "oscarlistnonservercontacts.moc"
