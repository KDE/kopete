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

class KopeteMessageManager;
class QTextEdit;


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
		KopeteMessageManager *msgManager() ;

		/**
		 * @return The KopeteView::ViewType of the view
		 */
		KopeteMessage::MessageType viewType();

		/**
		 * SIGNAL Emitted when the message is sent.
		 * This signal fires *BEFORE* the KopeteMessageManager::messageSent() signal.
		 * The window remains open until the KopeteMessageManager::messageSentSuccessfully()
		 * signal is emitted
		 */
		virtual void messageSent( KopeteMessage & ) = 0;

		/**
		 * SLOT called to pass a received message to the view.
		 */
		virtual void messageReceived( KopeteMessage & ) = 0;

		/**
		 * SIGNAL Emitted when the chat view is shown.
		 * FIXME: IS THIS IN USE ANYWHERE?
		 */
		virtual void shown() = 0;

		/**
		 * SLOT Sends the view's current message.
		 */
		virtual void sendMessage() = 0;

		/**
		 * SIGNAL Emitted when the view is closing.
		 */
		virtual void closing( KopeteView * ) = 0;

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
		 * SIGNAL Emitted when the view is activated ( raised by the user )
		 */
		virtual void activated( KopeteView * ) = 0;

		/**
		 * Returns the widget used to compose messages
		 */
		virtual QTextEdit *editWidget() = 0;

		/**
		 * Returns the view widget this class is wrapping
		 */
		virtual QWidget *mainWidget() = 0;

		/**
		 * SLOT to inform the view that the message was sent successfully.
		 */
		virtual void messageSentSuccessfully() = 0;

	protected:
		KopeteMessageManager *m_manager;
		KopeteMessage::MessageType m_type;
};

#endif
