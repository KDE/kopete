/*
    jabberawaydialog.cpp - Away dialog for Jabber

    Copyright (c) 2003 by Till Gerken <till@tantalo.net>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
#include "jabberaccount.h"
#include "jabberprotocol.h"
#include "jabberawaydialog.h"

JabberAwayDialog::JabberAwayDialog(JabberAccount *account, QWidget* parent, const char* name):
									KopeteAwayDialog(parent, name)
{

	m_account = account;

}

JabberAwayDialog::~JabberAwayDialog()
{
}

void JabberAwayDialog::setAway(int awayType)
{
/*
	switch(awayType)
	{
		case JabberProtocol::JabberAway:
				m_account->setPresence(m_account->protocol()->JabberKOSAway, getSelectedAwayMessage());
				break;
		case JabberProtocol::JabberXA:
				m_account->setPresence(m_account->protocol()->JabberKOSXA, getSelectedAwayMessage());
				break;
		case JabberProtocol::JabberDND:
				m_account->setPresence(m_account->protocol()->JabberKOSDND, getSelectedAwayMessage());
				break;
		default:
				kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Warning: Unknown away type!" << endl;
				break;
	}
*/
}

#include "jabberawaydialog.moc"
