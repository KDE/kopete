/*
    kopeteview.h - View Manager

    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>
    Copyright (c) 2004      by Matt Rogers           <matt.rogers@kdemail.net>
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

#include <kopete_export.h>

class Kopete::ChatSession;
class QTextEdit;
class Kopete::Message;


/**
 * @author Jason Keirstead
 *
 * Abstract parent class for all types of views used for messaging.  At present
 * these may be either KopeteView::Chat or KopeteView::Email.
 *
 */
class KOPETE_EXPORT KopeteView
{
	public:
		/**
		 * constructor
		 */
		KopeteView( Kopete::ChatSession *manager );

		/**
		 * @brief Returns the message currently in the edit area
		 * @return The Kopete::Message object containing the message
		 */
		virtual Kopete::Message currentMessage() = 0;
		/**
		 * Set the message that the view is currently editing.
		 * @param newMessage The Kopete::Message object containing the message to be edited.
		 */
		virtual void setCurrentMessage( const Kopete::Message &newMessage ) = 0;

		/**
		 * @brief Get the message manager
		 * @return The Kopete::ChatSession that the view is in communication with.
		 */
		Kopete::ChatSession *msgManager() const;

		/**
		 * @brief Get the view type
		 * @return The KopeteView::ViewType of the view
		 */
		Kopete::Message::ViewType viewType();

		/**
		 * @brief add a message to the view
		 *
		 * The message gets added at the end of the view and is automatically
		 * displayed. Classes that inherit from KopeteView should make this a slot.
		 */
		virtual void appendMessage( Kopete::Message & ) = 0;

		/**
		 * @brief append multiple messages to the view
		 *
		 * This function does the same thing as the above function but
		 * can be reimplemented if it is faster to apend several messages
		 * in the same time.
		 *
		 * The default implementation just call @ref appendMessage() X times
		 */
		virtual void appendMessages( QValueList<Kopete::Message> );

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
		 * a pointer to the Kopete::ChatSession given in the constructor
		 */
		Kopete::ChatSession *m_manager;
		/**
		 * the type of this view
		 */
		Kopete::Message::ViewType m_type;
};

#endif
