/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>
    Copyright (C) 2008-2009 Pali Rohár <pali.rohar@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/
#ifndef SKYPEADDCONTACT_H
#define SKYPEADDCONTACT_H

#include <addcontactpage.h>

class SkypeAddContactPrivate;
class SkypeProtocol;
class SkypeAccount;

/**
 * @author Michal Vaner (vorner)
 * @author Pali Rohár
 * Just a widget with a line and label ;-)
 */
class SkypeAddContact : public AddContactPage
{
	Q_OBJECT
	private:
		///internal things
		SkypeAddContactPrivate *d;
	public:
		/**
		 * Constructor.
		 * @param protocol Pointer to the Skype protocol.
		 * @param parent Widget inside which I will be showed.
		 * @param name My name I can be found by.
		 */
		SkypeAddContact(SkypeProtocol *protocol, QWidget *parent, SkypeAccount *account, const char *name);
		/**
		 * Destructor.
		 */
		~SkypeAddContact();
		/**
		 * Check, weather user wrote something sane.
		 * @return True if it is useable, false otherwise.
		 */
		virtual bool validateData();
	public slots:
		/**
		 * Adds it into the account.kdDebug(14311) << k_funcinfo << endl;//some debug info
		 * @param account Where to add it.
		 * @param metaContact Metacontact which will hold it.
		 * @return True if it worked, false if not.
		 */
		virtual bool apply(Kopete::Account *account, Kopete::MetaContact *metaContact);
};

#endif
