/***************************************************************************
                          kopetecontact.h  -  Kopete Contact
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KOPETECONTACT_H
#define KOPETECONTACT_H

#include "contactlist.h"

#include <qobject.h>

class QString;
class KopeteEvent;

/**
* This class abstracts a generic contact/buddie.
* Use it for inserting contacts in the contact list for example.
*/

class KopeteContact : public QObject
{
	Q_OBJECT
	public:
		/**
		* Contact's status
		**/
		enum ContactStatus { Online, Away, Offline };

		/**
		* Usually a contact is owned by a protocol plugin
		**/
		KopeteContact(QObject *parent);
		~KopeteContact();

		/**
		* set name of an KopeteContact
		* This is the string that gets drawn in the listview
		**/
		virtual void setName( const QString &name );
		/**
		* return name of an KopeteContact
		**/
		virtual QString name() const;
		/**
		* User ID of a KopeteContact
		**/
		virtual QString userID() const;
		/**
		* return status of an KopeteContact
		**/
		virtual ContactStatus status() const;
		/**
		* The text describing the contact's status
		* The default implement does what you'd expect,
		* but you might want to reimplement it for more
		* fine-grained reporting of status
		**/
		virtual QString statusText() const;
		/**
		* The name of the icon associated with the contact's status
		**/
		virtual QString statusIcon() const;
		/**
		* The "importance" of this contact, used for sorting
		* This is almost always related to the contact's status
		* Here is how ICQ does it:
		* 25 = Free For Chat
		* 20 = Online
		* 15 = Away (temporary away)
		* 10 = Not Available (extended away)
		* 5 = Invisible
		* 0 = Offline
		*
		* The default implementation returns 20 for Online,
		* 10 for away, and 0 for offline
		* Please make your importance values between 0 and 25,
		* and try to follow ICQ's scheme somewhat
		**/
		virtual int importance() const;
		/**
		* This should typically pop up a KopeteChatWindow
		**/
		virtual void execute() {}
		/**
		* Show a context menu of actions pertaining to this contact
		* I hate having the group parameter, but its used for when
		* a contact can be in multiple groups and you have to move
		* a specific instance from one group to another.
		**/
		virtual void showContextMenu(QPoint, QString) {}

	signals:
		/**
		* Connect to this signal to know when the contact
		* changed its status
		**/
		void statusChanged();
		/**
		* Connect to this signal to know when the contact
		* changed its name/nick
		**/
		void nameChanged(const QString &name);
		/**
		* This gets emitted usually when you receive a message from
		* this contact.
		**/
		void incomingEvent(KopeteEvent *);
		
	private:
		QString mName;
		QString mUserID;
};
#endif
