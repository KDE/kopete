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

#include <QtGui/QWidget>

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
} //Global::UI

} //UI

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

