/*
    kopetetproperties.cpp - Kopete Properties

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteproperties.h"

#include <kdebug.h>

#include <qdom.h>
#include <qvariant.h>
#include <typeinfo>

namespace Kopete {
namespace Properties {

// Keep as much type-independent code out of the templated stuff as we can
// FIXME: shouldn't be inline
void customPropertyDataIncorrectType( const char *name, const std::type_info &found, const std::type_info &expected )
{
	kWarning(14010) << "data time mismatch for property data name " << name
		<< ". found: " << found.name() << ", expected: " << expected.name() << endl;
}

template<>
int variantTo<int>(QVariant) { return 0; }
//...

QVariant variantFromXML(const QDomElement&)
{
	return QVariant();
}

void variantToXML(QVariant v, QDomElement &)
{
	Q_UNUSED(v);
}

} // namespace Properties
} // namespace Kopete
