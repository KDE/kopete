/*
    loadmovie.h - Load movie from themes in KDE 3.1

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

#ifndef COMPAT_LOADMOVIE_H
#define COMPAT_LOADMOVIE_H

#include <kicontheme.h>

class QString;
class QMovie;

namespace KopeteCompat
{

/**
* @brief Loads an animated icon.
* Loads an animated icon from the crystalsvg set, where kopete installs its
* animations. Note that user-supplied icon themes overriding the kopete
* provided ones will be ignored by this function.
* @param name the name of the icon
* @param group the icon group; see KIconLoader::loadIcon()
* @return a QMovie object. Can be null if not found.
*
* @author Richard Smith <richard@metafoo.co.uk>
*/
QMovie loadMovie( const QString &name, KIcon::Group group );

}

#endif
