/*
    kopeteuiglobal.h - Kopete UI Globals

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEUIGLOBAL_H
#define KOPETEUIGLOBAL_H

#include <qwidget.h>

#include "kopete_export.h"

namespace Kopete
{

namespace UI
{

/**
 * This namespace contains the Kopete user interface's global settings
 */
namespace Global
{
	/**
	 * Set the main widget to widget
	 */
	KOPETE_EXPORT void setMainWidget( QWidget *widget );
	/**
	 * Returns the main widget - this is the widget that message boxes
	 * and KNotify stuff should use as a parent.
	 */
	KOPETE_EXPORT QWidget *mainWidget();

	/**
	 * \brief Returns the WId of the system tray.
	 *
	 * Allows developers easy access to the WId of the system tray so
	 * that it can be used for passive popups in the protocols
	 * \return the WId of the system tray. Returns the WId of the main
	 * widget if there's no system tray.
	 */ 
	KOPETE_EXPORT int sysTrayWId();

	/**
	 * \brief Set the WId of the system tray.
	 *
	 * Called by the KopeteSystemTray constructor and destructor to 
	 * set the WId for the system tray appropriately
	 */
	KOPETE_EXPORT void setSysTrayWId( int newWinId );
} //Global::UI

} //UI

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

