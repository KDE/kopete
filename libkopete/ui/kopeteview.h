/*
    kopeteview.h - View Manager

    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>
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


#ifndef KOPETEVIEW_H
#define KOPETEVIEW_H

#include "kopetemessage.h"
#include <qvaluelist.h>

class KopeteMessageManager;
class QTextEdit;
class KopeteMessage;


/**
 * @author Jason Keirstead
 *
 * Abstract parent class for all types of views used for messaging.  At present
 * these may be either KopeteView::Chat or KopeteView::Email.
 *
 */
class KopeteView
{
	public:
		/**
		 * @brief constructor
		 */
		KopeteView( KopeteMessageManager *manager );

		/**
		 * Returns the message currently in the edit area
		 * @return The message
		 */
		virtual KopeteMessage currentMessage() = 0;
		/**
		 * Set the message that the view is currently editing.
		 * @param newMessage The new message to be edited.
		 */
		virtual void setCurrentMessage( const KopeteMessage &newMessage ) = 0;

		/**
		 * @return The KopeteMessageManager that the view is in communication
		 * with.
		 */
		KopeteMessageManager *msgManager();
		/**
		 * same function as above
		 */
		const KopeteMessageManager *msgManager() const;

		/**
		 * @return The KopeteView::ViewType of the view
		 */
		KopeteMessage::MessageType viewType();

		/**
		 * SLOT append a message (and show it) to the view
		 */
		virtual void appendMessage( KopeteMessage & ) = 0;
		/**
		 * Same function as above, but append multiple message.
		 * The default implementation just call @ref appendMessage() X times
		 * but you can reimplement it if it is faster to apend several messages
		 * in the same time
		 */
		virtual void appendMessages( QValueList<KopeteMessage> );

		/**
		 * SLOT Sends the view's current message.
		 */
		//virtual void sendMessage() = 0;

		/**
		 * Raises the view to the top of the screen.
		 * @param activate change the focus to the window
		 */
		virtual void raise(bool activate=false) = 0;

		/**
		 * Clears the buffer
		 */
		 virtual void clear();

		/**
		 * Make the view visible if it is currently hidden.
		 */
		virtual void makeVisible() = 0;

		/*
		 * Close this view
		 */
		virtual bool closeView( bool force = false ) = 0;

		/**
		 * Is the view currently visible.
		 * @return Whether the view is visible or not.
		 */
		virtual bool isVisible() = 0;

		/**
		 * Returns the widget used to compose messages
		 */
		virtual QTextEdit *editWidget() = 0;

		/**
		 * Returns the view widget this class is wrapping
		 * Can be simply reimplemented to return this if this is a widget
		 */
		virtual QWidget *mainWidget() = 0;

		/**
		 * SLOT to inform the view that the message was sent successfully.
		 */
		virtual void messageSentSuccessfully() = 0;

		/**
		 * SIGNAL Emitted when the message is sent.
		 * This signal fires *BEFORE* the KopeteMessageManager::messageSent() signal.
		 * The window remains open until the KopeteMessageManager::messageSentSuccessfully()
		 * signal is emitted
		 */
		virtual void messageSent( KopeteMessage & ) = 0;

		/**
		 * SIGNAL Emitted when the view is closing.
		 */
		virtual void closing( KopeteView * ) = 0;

		/**
		 * SIGNAL Emitted when the view is activated ( raised by the user )
		 */
		virtual void activated( KopeteView * ) = 0;


	protected:
		/**
		 * a pointer to the KopeteMessageManager given in the constructor
		 */
		KopeteMessageManager *m_manager;
		KopeteMessage::MessageType m_type;
};

#endif
