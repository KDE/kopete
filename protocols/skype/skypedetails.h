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
#ifndef SKYPEDETAILS_H
#define SKYPEDETAILS_H

#include <ui_skypedetailsbase.h>

class QString;
class SkypeAccount;
namespace Ui { class SkypeDetailsBase; }

/**
 * @author Michal Vaner (VORNER) <michal.vaner@kdemail.net>
 * @author Pali Rohár
 * Dialog that shows users details
 */
class SkypeDetails : public KDialog, private Ui::SkypeDetailsBase {
	Q_OBJECT
	private:
		SkypeAccount *account;
	protected slots:
		void changeAuthor(int item);
	protected:
		///Make sure it is deleted after it is closed
		void closeEvent(QCloseEvent *e);
		///Main dialog widget
		Ui::SkypeDetailsBase *dialog;
	public:
		///Just constructor
		SkypeDetails();
		///Only a destructor
		~SkypeDetails();
	public slots:
		/**
		 * Sets the ID, the nick and the name
		 * @param id The ID of the user
		 * @param nick user's nick
		 * @param name user's full name
		 */
		SkypeDetails &setNames(const QString &id, const QString &nick, const QString &name);
		/**
		 * Sets the phone numbers what will be showed
		 * @param priv The private phone number
		 * @param mobile The mobile phone
		 * @param work The work phone
		 */
		SkypeDetails &setPhones(const QString &priv, const QString &mobile, const QString &work);
		/**
		 * Sets the homepage
		 * @param homepage The value to set
		 */
		SkypeDetails &setHomepage(const QString &homepage);
		/**
		 * Sets the users authorization
		 * @param author The authorization - 0 = authorized, 1 = not authorized, 2 = blocked
		 */
		SkypeDetails &setAuthor(int author, SkypeAccount *account);
		/**
		 * Sets the string to show in 'sex' edit box
		 */
		SkypeDetails &setSex(const QString &sex);
};

#endif
