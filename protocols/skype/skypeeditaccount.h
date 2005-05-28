/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>

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

#ifndef SKYPEEDITACCOUNT_H
#define SKYPEEDITACCOUNT_H

#include <ui/skypeeditaccountbase.h>
#include "editaccountwidget.h"

class SkypeEditAccountPrivate;
class SkypeProtocol;

/**
 * @author Michal Vaner
 * @short Skype account edit-widget
 * This widget will be showed inside the add account wizard when adding skype account or in edit account dialog, when editing skype account.
 */
class skypeEditAccount : public SkypeEditAccountBase, public KopeteEditAccountWidget
{
Q_OBJECT
	private:
		///Some internal things
		SkypeEditAccountPrivate *d;
	protected slots:
		///User wants to launch skype now
		virtual void testLaunch();
	public:
		/**
		 * Constructor.
		 * @param account The account we are editing. 0 if new should be created.
		 * @param parent Inside what it will be showed.
		 */
		skypeEditAccount(SkypeProtocol *protocol, Kopete::Account *account, QWidget *parent = 0L);
		/**
		 * Destructor.
		 */
		virtual ~skypeEditAccount();
		/**
		 * Check, weather the written data can be used.
		 * @return True if the data are OK, false if not.
		 */
		virtual bool validateData();
		/**
		 * Aply all changes. Will change the actual account or create new one, if no was given.
		 * @return Pointer to that account.
		 */
		virtual Kopete::Account *apply();
	public slots:
};

#endif

