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

#include <kopeteplugin.h>

#include "kopetemessage.h"

namespace Kopete
{
class MessageManager;
class Protocol;
class Contact;
class MessageEvent;
}

class KopeteView;
class QTextEdit;



struct KopeteViewManagerPrivate;

/**
 * Relates an actual chat to the means used to view it.
 */
class KopeteViewManager : public Kopete::Plugin
{
	Q_OBJECT
	public:
		/** This is a singleton class.  Call this method to get a pointer to
		 * a KopeteViewManager.
		 */
		static KopeteViewManager *viewManager();

		KopeteViewManager( QObject *parent, const char *name, const QStringList &args );
		~KopeteViewManager();

		/**
		 * Return a view for the supplied Kopete::MessageManager.  If one already
		 * exists, it will be returned, otherwise, a new view is created.
		 * @param manager The Kopete::MessageManager we are viewing.
		 * @param foreignMessage Whether the message is inbound or outbound.
		 * @param type Specifies the type of view.
		 */
		KopeteView *view( Kopete::MessageManager *, bool foreignMessage, Kopete::Message::ViewType type = Kopete::Message::Undefined );

		/**
		 * Provide access to the list of KopeteChatWindow the class maintains.
		 */
		KopeteView *activeView() const;

	private:


		KopeteViewManagerPrivate *d;
		static KopeteViewManager *s_viewManager;

	public slots:
		/**
		 * Make a view visible and on top.
		 * @param manager The originating Kopete::MessageManager.
		 * @param foreignMessage Whether the message is inbound or outbound.
		 */
		void readMessages( Kopete::MessageManager*, bool outgoingMessage );

		/**
		 * Called when a new message has been appended to the given
		 * Kopete::MessageManager.  Procures a view for the message, and generates any notification events or displays messages, as appropriate.
		 * @param msg The new message
		 * @param manager The originating Kopete::MessageManager
		 */
		void messageAppended( Kopete::Message &msg, Kopete::MessageManager *manager);

		void nextEvent();

	private slots:
		void slotViewDestroyed( KopeteView *);
		void slotMessageManagerDestroyed( Kopete::MessageManager * );

		/**
		 * An event has been deleted.
		 */
		void slotEventDeleted( Kopete::MessageEvent * );

		void slotPrefsChanged();
		void slotViewActivated( KopeteView * );

		void slotRequestView(KopeteView*& , Kopete::MessageManager * , Kopete::Message::ViewType type );

		//obsolete, used only by spellchecking plugin
		void slotGetActiveView(KopeteView*&);
};

#endif
