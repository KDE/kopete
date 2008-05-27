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

#include <QtGui/QWidget>

#include "../kopetemessage.h"
#include "../kopete_export.h"

namespace Kopete
{
	class ViewPlugin;
}

/**
 * @author Jason Keirstead
 *
 * Abstract parent class for all types of views used for messaging.These view objects
 * are provided by a @ref Kopete::ViewPlugin
 *
 * @see Kopete::ViewPlugin
 */
class KOPETE_EXPORT KopeteView
{
	public:
		/**
		 * constructor
		 */
		KopeteView( Kopete::ChatSession *manager, Kopete::ViewPlugin *parent );
		virtual ~KopeteView();

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
		virtual void appendMessages( QList<Kopete::Message> );

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

		/**
		 * @brief Register a handler for the context menu
		 *
		 * Plugins should call this slot at view creation to register
		 * themselves as handlers for the context menu of this view. Plugins
		 * can attach to the viewCreated signal of KopeteMessageManagerFactory
		 * to know when views are created.
		 *
		 * A view does not need to implement this method unless they have context
		 * menus that can be extended
		 *
		 * @param target A target QObject for the contextMenuEvent signal of the view
		 * @param slot A slot that matches the signature ( QString&, KMenu *)
		 */
		virtual void registerContextMenuHandler( QObject *target, const char*slot ){ Q_UNUSED(target); Q_UNUSED(slot); }

		/**
		* @brief Register a handler for the tooltip
		*
		* Plugins should call this slot at view creation to register
		* themselves as handlers for the tooltip of this view. Plugins
		* can attach to the viewCreated signal of KopeteMessageManagerFactory
		* to know when views are created.
		*
		* A view does not need to impliment this method unless it has the ability
		* to show tooltips
		*
		* @param target A target QObject for the contextMenuEvent signal of the view
		* @param slot A slot that matches the signature ( QString&, KMenu *)
		*/
		virtual void registerTooltipHandler( QObject *target, const char*slot ){ Q_UNUSED(target); Q_UNUSED(slot); }

		/**
		 * @brief Returns the Kopete::ViewPlugin responsible for this view
		 *
		 * KopeteView objects are created by plugins. This returns a pointer to the plugin
		 * that created this view. You can use this to infer other information on this view
		 * and it's capabilities.
		 */
		Kopete::ViewPlugin *plugin();

	protected:
		/**
		 * a pointer to the Kopete::ChatSession given in the constructor
		 */
		Kopete::ChatSession *m_manager;
		Kopete::ViewPlugin *m_plugin;
};

#endif
