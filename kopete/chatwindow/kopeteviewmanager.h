/*
    kopeteviewmanager.h - View Manager

    Copyright (c) 2003 by Jason Keirstead
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEVIEWMANAGER_H
#define KOPETEVIEWMANAGER_H

#include <qobject.h>

#include "kopetemessage.h"

#define NEW_WINDOW 0
#define GROUP_BY_PROTOCOL 1
#define GROUP_ALL 2


class KopeteMessageManager;
class KopeteProtocol;
class KopeteContact;
class KopeteEvent;
class KopeteView;
class QTextEdit;



struct KopeteViewManagerPrivate;

/**
 * Relates an actual chat to the means used to view it.
 */
class KopeteViewManager : public QObject
{
	Q_OBJECT
	public:
		/** This is a singleton class.  Call this method to get a pointer to
		 * a KopeteViewManager.
		 */
		static KopeteViewManager *viewManager();

		~KopeteViewManager();

		/**
		 * Return a view for the supplied KopeteMessageManager.  If one already
		 * exists, it will be returned, otherwise, a new view is created.
		 * @param manager The KopeteMessageManager we are viewing.
		 * @param foreignMessage Whether the message is inbound or outbound.
		 * @param type Specifies the type of view.
		 */
		KopeteView *view( KopeteMessageManager *, bool foreignMessage, KopeteMessage::MessageType type = KopeteMessage::Undefined );

		/**
		 * Provide access to the list of KopeteChatWindow the class maintains.
		 */
		KopeteView *activeView() const;

	private:
		/**
	 	* Private constructor: we are a singleton
	 	*/
		KopeteViewManager();

		KopeteViewManagerPrivate *d;
		static KopeteViewManager *s_viewManager;

	public slots:
		/**
		 * Make a view visible and on top.
		 * @param manager The originating KopeteMessageManager.
		 * @param foreignMessage Whether the message is inbound or outbound.
		 */
		void readMessages( KopeteMessageManager*, bool outgoingMessage );

		/**
		 * Called when a new message has been appended to the given
		 * KopeteMessageManager.  Procures a view for the message, and generates any notification events or displays messages, as appropriate.
		 * @param msg The new message
		 * @param manager The originating KopeteMessageManager
		 */
		void messageAppended( KopeteMessage &msg, KopeteMessageManager *manager);

		void nextEvent();

	private slots:
		void slotViewDestroyed( KopeteView *);
		void slotMessageManagerDestroyed( KopeteMessageManager * );


		/**
		 * An event has been deleted. 
		 */
		void slotEventDeleted( KopeteEvent * );

		void slotPrefsChanged();
		void slotViewActivated( KopeteView * );

		void slotRequestView(KopeteView*& , KopeteMessageManager * , KopeteMessage::MessageType type );

		//obsolete, used only by spellchecking plugin
		void slotGetActiveView(KopeteView*&);

	signals:
		/**
		 * A new message is received, but not yet shown in the chatwindow
	 	 */
		void newMessageEvent(KopeteEvent *);
};

#endif
