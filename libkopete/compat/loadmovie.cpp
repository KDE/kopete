/*
    loadmovie.cpp - Load movie from themes in KDE 3.1

    Copyright (c) 2003      by Richard Smith          <richard@metafoo.co.uk>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "loadmovie.h"

#include <qmovie.h>
#include <kconfig.h>
#include <kglobal.h>

// retrieve the size the user wants icons of a certain group to be
static int iconSize( KIcon::Group group )
{
	// These have to match the order in kicontheme.h
	static const char * const groups[] = { "Desktop", "Toolbar", "MainToolbar", "Small", "Panel" };
	KConfig *config = KGlobal::config();

	if( (unsigned)group >= (sizeof(groups)/sizeof(groups[0])) )
		return 0;

	QString groupName = QString::fromLatin1( groups[group] ) + "Icons";
	KConfigGroupSaver cs( config, groupName );
	return config->readNumEntry( "Size", 0 );
}

// Load one of the Kopete-provided animated icons from Crystal SVG;
// KIconLoader::loadMovie broken in KDE <= 3.1 if current icon set is not crystal
QMovie KopeteCompat::loadMovie( const QString &name, KIcon::Group group )
{
	QString file = name + ".mng";
	QString appname = KGlobal::instance()->instanceName();
	KIconTheme crystal( QString::fromLatin1("crystalsvg"), appname );

	KIcon icon = crystal.iconPath( file, iconSize( group ), KIcon::MatchExact );
	if( !icon.isValid() )
		icon = crystal.iconPath( file, iconSize( group ), KIcon::MatchBest );
	if( !icon.isValid() )
		return QMovie();

	return QMovie( icon.path );
}
