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

#ifndef IMCONTACT_H
#define IMCONTACT_H

#include <improtocol.h>
#include <contactlist.h>

#include <qobject.h>

class QString;
class KListViewItem;
class QListView;
class QListViewItem;
class QTimer;
/**
  *@author duncan
  */

class KopeteContact : public QObject, public KListViewItem
{
	Q_OBJECT
	public:
		KopeteContact(QListViewItem *parent);
		KopeteContact(QListView *parent);
		~KopeteContact();

		void setHidden ( bool hideme); // hides and shows a contact
		bool isHidden();				// returns if a contact is hidden
		
		// makes a contact blink to notify the user
		// that something special happened
		void triggerNotify();

		// set name of an KopeteContact
		// This is the string that gets drawn in the listview
		virtual void setName( const QString &name );
		// return name of an KopeteContact
		virtual QString name(void) const;

		// This should typically pop up a KopeteChatWindow
		virtual void execute() {}
		// Show a context menu of actions pertaining to this contact
		virtual void showContextMenu(QPoint) {}

	signals:
		void statusChanged();
		void nameChanged();
		
	private:
		void paintCell (QPainter *p, const QColorGroup &cg, int column, int width, int alignment);
		QTimer *blinkTimer;
		int numBlinks;
		QString mName;

	private slots:
		void slotNotify(void);
};
#endif
