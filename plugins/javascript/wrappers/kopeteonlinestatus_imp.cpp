/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>

#include "kopetecontact.h"
#include "kopeteonlinestatus_imp.h"

Status::Status( const KopeteOnlineStatus &s, QObject *parent,  const char *name )
	: BindingObject( parent, name ), status( const_cast<KopeteOnlineStatus*>(&s) ) {}

#include "kopeteonlinestatus_imp.moc"
