/*
    kopeteviewplugin.cpp - View Manager

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

#include "kopeteviewplugin.h"

Kopete::ViewPlugin::ViewPlugin( const KAboutData &instance, QObject *parent ) :
	Kopete::Plugin( instance, parent )
{

}

Kopete::ViewPlugin::ViewPlugin( QObject* parent ): Plugin( parent )
{

}


void Kopete::ViewPlugin::aboutToUnload()
{
	emit readyForUnload();
}
