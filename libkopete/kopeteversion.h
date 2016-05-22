/*
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef _KOPETE_VERSION_H_
#define _KOPETE_VERSION_H_

#define KOPETE_VERSION_MAJOR 1
#define KOPETE_VERSION_MINOR 9
#define KOPETE_VERSION_RELEASE 2

#define KOPETE_STRINGIFY(a) #a
#define KOPETE_MAKE_VERSION_STRING( a,b,c ) KOPETE_STRINGIFY(a.b.c)
#define KOPETE_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

#define KOPETE_VERSION_STRING \
  KOPETE_MAKE_VERSION_STRING(KOPETE_VERSION_MAJOR,KOPETE_VERSION_MINOR,KOPETE_VERSION_RELEASE)
#define KOPETE_VERSION \
  KOPETE_MAKE_VERSION(KOPETE_VERSION_MAJOR,KOPETE_VERSION_MINOR,KOPETE_VERSION_RELEASE)

#define KOPETE_IS_VERSION(a,b,c) ( KOPETE_VERSION >= KOPETE_MAKE_VERSION(a,b,c) )


#endif // _KOPETE_VERSION_H_
