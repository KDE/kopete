/***************************************************************************
                          kopetecontact.h  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
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

#include <contactlist.h>

#include <qobject.h>

class QString;
class KopeteEvent;

/**
  *@author duncan
  */

class KopeteContact : public QObject
{
	Q_OBJECT
	public:
		enum ContactStatus { Online, Away, Offline };

		KopeteContact(QObject *parent);
		~KopeteContact();

		// set name of an KopeteContact
		// This is the string that gets drawn in the listview
		virtual void setName( const QString &name );
		// return name of an KopeteContact
		virtual QString name() const;

		virtual ContactStatus status() const;
		// The text describing the contact's status
		// The default implement does what you'd expect,
		// but you might want to reimplement it for more
		// fine-grained reporting of status
		virtual QString statusText() const;
		// The name of the icon associated with the contact's status
		virtual QString statusIcon() const;

		// The "importance" of this contact, used for sorting
		// This is almost always related to the contact's status
		// Here is how ICQ does it: 
		// 25 = Free For Chat
		// 20 = Online
		// 15 = Away (temporary away)
		// 10 = Not Available (extended away)
		// 5 = Invisible
		// 0 = Offline

		// The default implementation returns 20 for Online,
		// 10 for away, and 0 for offline
		// Please make your importance values between 0 and 25,
		// and try to follow ICQ's scheme somewhat
		virtual int importance() const;

		// This should typically pop up a KopeteChatWindow
		virtual void execute() {}
		// Show a context menu of actions pertaining to this contact
		virtual void showContextMenu(QPoint) {}

	signals:
		void statusChanged();
		void nameChanged(const QString &name);
		void incomingEvent(KopeteEvent *);
		
	private:
		QString mName;
};
#endif
