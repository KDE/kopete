/*
    kopeteview.h - View Manager

    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
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
		 * constructor
		 */
		KopeteView( KopeteMessageManager *manager );

		/**
		 * @brief Returns the message currently in the edit area
		 * @return The KopeteMessage object containing the message
		 */
		virtual KopeteMessage currentMessage() = 0;
		/**
		 * Set the message that the view is currently editing.
		 * @param newMessage The KopeteMessage object containing the message to be edited.
		 */
		virtual void setCurrentMessage( const KopeteMessage &newMessage ) = 0;

		/**
		 * @brief Get the message manager
		 * @return The KopeteMessageManager that the view is in communication with.
		 */
		KopeteMessageManager *msgManager();
		/**
		 * const version of the above function
		 */
		const KopeteMessageManager *msgManager() const;

		/**
		 * @brief Get the view type
		 * @return The KopeteView::ViewType of the view
		 */
		KopeteMessage::MessageType viewType();

		/**
		 * @brief add a message to the view
		 *
		 * The message gets added at the end of the view and is automatically
		 * displayed. Classes that inherit from KopeteView should make this a slot.
		 */
		virtual void appendMessage( KopeteMessage & ) = 0;
		/**
		 * @brief append multiple messages to the view
		 *
		 * This function does the same thing as the above function but
		 * can be reimplemented if it is faster to apend several messages
		 * in the same time.
		 *
		 * The default implementation just call @ref appendMessage() X times
		 */
		virtual void appendMessages( QValueList<KopeteMessage> );

		/**
		 * @brief Raises the view above other windows
		 * @param activate change the focus to the window
		 */
		virtual void raise(bool activate = false) = 0;

		/**
		 * @brief Clear the buffer
		 */
		 virtual void clear();

		/**
		 * @brief Make the view visible
		 *
		 * Makes the view visible if it is currently hidden.
		 */
		virtual void makeVisible() = 0;

		/**
		 * @brief Close this view
		 */
		virtual bool closeView( bool force = false ) = 0;

		/**
		 * @brief Get the current visibility of the view
		 * @return Whether the view is visible or not.
		 */
		virtual bool isVisible() = 0;

		/**
		 * @brief Get the widget used to compose messages
		 * @return The QTextEdit object used to compose messages
		 */
		virtual QTextEdit *editWidget() = 0;

		/**
		 * @brief Get the view widget
		 *
		 * Can be reimplemented to return this if derived object is a widget
		 */
		virtual QWidget *mainWidget() = 0;

		/**
		 * @brief Inform the view the message was sent successfully
		 *
		 * This should be reimplemented as a SLOT in any derived objects
		 */
		virtual void messageSentSuccessfully() = 0;


	protected:
		/**
		 * a pointer to the KopeteMessageManager given in the constructor
		 */
		KopeteMessageManager *m_manager;
		/**
		 * the type of this view
		 */
		KopeteMessage::MessageType m_type;
};

#endif
