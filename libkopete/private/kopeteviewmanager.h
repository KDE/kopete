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

#include "kopetemessage.h"
#include "kopete_export.h"

namespace Kopete
{
	class ChatSession;
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
class KOPETE_EXPORT KopeteViewManager : public QObject
{
	Q_OBJECT
	public:
		/** This is a singleton class.  Call this method to get a pointer to
		 * a KopeteViewManager.
		 */
		static KopeteViewManager *viewManager();

		KopeteViewManager();
		~KopeteViewManager();

		/**
		 * Return a view for the supplied Kopete::ChatSession.  If one already
		 * exists, it will be returned, otherwise, a new view is created.
		 * @param session The Kopete::ChatSession we are viewing.
		 * @param requestedPlugin Specifies the view plugin to use.
		 */
		KopeteView *view( Kopete::ChatSession *session, const QString &requestedPlugin = QString::null );

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
		 * @param manager The originating Kopete::ChatSession.
		 * @param outgoingMessage Whether the message is inbound or outbound.
		 * @param activate Indicate whether the view should be activated
		 * @todo Document @p activate
		 */
		void readMessages( Kopete::ChatSession* manager, bool outgoingMessage, bool activate = false );

		/**
		 * Called when a new message has been appended to the given
		 * Kopete::ChatSession.  Procures a view for the message, and generates any notification events or displays messages, as appropriate.
		 * @param msg The new message
		 * @param manager The originating Kopete::ChatSession
		 */
		void messageAppended( Kopete::Message &msg, Kopete::ChatSession *manager);

		void nextEvent();

	private slots:
		void slotViewDestroyed( KopeteView *);
		void slotChatSessionDestroyed( Kopete::ChatSession * );

		/**
		 * An event has been deleted.
		 */
		void slotEventDeleted( Kopete::MessageEvent * );

		void slotPrefsChanged();
		void slotViewActivated( KopeteView * );
};

#endif
