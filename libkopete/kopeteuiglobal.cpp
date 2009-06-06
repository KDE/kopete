/*
    kopeteuiglobal.cpp - Kopete UI Globals

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

#include "kopeteuiglobal.h"

#include <qpointer.h>


namespace Kopete
{


namespace
{
	QPointer<QWidget> g_mainWidget;
}

void UI::Global::setMainWidget( QWidget *widget )
{
	g_mainWidget = widget;
}

QWidget *UI::Global::mainWidget()
{
	return g_mainWidget;
}
}

// vim: set noet ts=4 sts=4 sw=4:

