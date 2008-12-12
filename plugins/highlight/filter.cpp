/*
    filter.cpp  -  filter for the highlight plugin

    Copyright (c) 2003      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include "filter.h"
#include <QRegExp>

Filter::Filter()
{
}

Filter::~Filter()
{
}

QString Filter::className() const
{
	QString cl="filter:"+displayName;
	return cl.replace( ' ' , '_' ).replace( '\\' , '_' ).replace( '/' , '_' )
			.replace( QRegExp("[\\x0000-\\x002C\\x003B-\\x0040\\x005B-\\x005E\\x007B-\\x00BF]") , "-" );
}
 
 
