/*
    Tests for the Kopete properties classes

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteproperties.h"

#include <qtextstream.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>

using namespace Kopete::Properties;

static QTextStream _out( stdout, IO_WriteOnly );

class PropertyHost : public WithProperties<PropertyHost> {};

class FooProperty : public SimpleDataProperty<PropertyHost, QString>
{
public:
	const char *name() const { return "foo"; }
} fooProperty;

int main( int argc, char *argv[] )
{
	PropertyHost myPropertyHost;
	Q_ASSERT( myPropertyHost.property(fooProperty).isNull() );
	myPropertyHost.setProperty( fooProperty, QString::fromLatin1("Foo!") );
	Q_ASSERT( myPropertyHost.property(fooProperty) == QString::fromLatin1("Foo!") );
}
