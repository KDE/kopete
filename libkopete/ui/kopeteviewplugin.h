/*
    kopeteviewplugin.h - View Manager

    Copyright (c) 2005      by Jason Keirstead       <jason@keirstead.org>
    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEVIEWPLUGIN_H
#define KOPETEVIEWPLUGIN_H

#include "kopeteplugin.h"

class KopeteView;

namespace Kopete
{

class ChatSession;

/**
 * @author Jason Keirstead
 *
 * @brief Factory plugin for creating KopeteView objects.
 *
 * Kopete ships with two of these currently, a Chat Window view plugin, and
 * an Email Window view plugin.
 *
 */
class KOPETE_EXPORT ViewPlugin : public Plugin
{
	public:
		/**
		 * @brief Create and initialize the plugin
		 */
		explicit ViewPlugin( const KComponentData &instance, QObject *parent = 0L );

		/**
		 * @brief Creates a view to be associated with the passed in session
		*/
		virtual KopeteView *createView( ChatSession * /*session*/ ){ return 0L; }

		/**
		 * @brief Reimplemented from Kopete::Plugin
		 */
		virtual void aboutToUnload();
};

}

#endif
