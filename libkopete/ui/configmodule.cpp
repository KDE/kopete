/*
    configmodule.cpp - Kopete Plugin Config Module

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett       <duncan@kde.org>

    Portions of this code based in Noatun plugin code:
    Copyright (c) 2000-2002 The Noatun Developers

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "configmodule.h"

#include <qlayout.h>

#include <kdeversion.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <stdlib.h>

#if QT_VERSION < 0x030102 && KDE_VERSION < KDE_MAKE_VERSION( 3, 1, 90 )
#include <kapplication.h>
#endif

ConfigModule::ConfigModule(const QString & /* name */, const QString & /* description */, QObject * /* owner */ )
: QWidget( 0L )
{
	kdWarning() << k_funcinfo << "Creating obsolete ConfigModule. This is no longer supported. Fix your plugin." << endl << kdBacktrace() << endl;
	abort();
}

ConfigModule::ConfigModule( const QString & /* name */, const QString & /* description */, const QString & /* pixmap */, QObject * /* owner */ )
: QWidget( 0L )
{
	kdWarning() << k_funcinfo << "Creating obsolete ConfigModule. This is no longer supported. Fix your plugin." << endl << kdBacktrace() << endl;
	abort();
}


ConfigModule::~ConfigModule()
{
#if QT_VERSION < 0x030102 && KDE_VERSION < KDE_MAKE_VERSION( 3, 1, 90 )
	// Due to a bug in Qt 3.1 and 3.1.1 no close events are sent to hidden
	// widgets, causing the KJanusWidget to crash. This workaround is
	// rather intrusive and should be used only in the affected versions
	// to avoid hard to track bugs in the future. KDE HEAD (to become 3.2)
	// has a workaround for this problem, and additionally it's fixed in
	// Qt 3.1.2.
	kapp->sendPostedEvents();
#endif
}

void ConfigModule::activate()
{
}

#include "configmodule.moc"

// vim: set noet ts=4 sts=4 sw=4:

