/*
    Kopete Oscar Protocol
    oscartypeclasses.h - Oscar Type Definitions

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>
    Copyright (c) 2004 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef _OSCARTYPECLASSES_H_
#define _OSCARTYPECLASSES_H_

#include <qglobal.h>
#include <qstring.h>
#include <QList>
#include "liboscar_export.h"

namespace Oscar
{
class LIBOSCAR_EXPORT TLV
{
public:

	TLV();
	TLV( quint16, quint16, char* data );
	TLV( quint16, quint16, const QByteArray& );
	TLV( const TLV& t );

	operator bool() const;

	quint16 type;
	quint16 length;
	QByteArray data;

};

}

//kate: indent-mode csands; auto-insert-doxygen on; tab-width 4;

#endif
