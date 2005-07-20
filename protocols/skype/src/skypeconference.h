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
#ifndef SKYPECONFERENCE_H
#define SKYPECONFERENCE_H

#include <qdialog.h>

class SkypeConferencePrivate;
class SkypeCallDialog;

class QString;

/**
 * @author Michal Vaner
 * @short Dialog to group calls
 * This dialog can group calls that belongs
 */
class SkypeConference : public QDialog
{
	Q_OBJECT
	private:
		///Here are stored the private things, just for better readibility
		SkypeConferencePrivate *d;
	protected:
		///Make a suicide when closed
		virtual void closeEvent(QCloseEvent *e);
	public:
		/**
		 * Constructor, also shows itself
		 * @param id My ID
		 */
		SkypeConference(const QString &id);
		///Destrucotr
		~SkypeConference();
		/**
		 * Add a call to this group
		 * @param dialog What to add there
		 */
		void embedCall(SkypeCallDialog *dialog);
	signals:
		/**
		 * The conference is being removed right now
		 * @param conferenceId what conference
		 */
		void removeConference(const QString &conferenceId);
};

#endif
