/*
    Tests for Kopete Properties

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Copyright (c) 2005      by Duncan Mac-Vicar       <duncan@kde.org>

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

#include "kopetepropertiestest.h"

#include <kunittest/module.h>

#include "kopeteproperties.h"

#include <qstring.h>
#include <qtextstream.h>

#include <kaboutdata.h>
#include <kglobal.h>
#include <kstandarddirs.h>

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kopetepropertiestest, "KopeteSuite");
KUNITTEST_MODULE_REGISTER_TESTER( KopetePropertiesTest );

using namespace Kopete::Properties;

static QTextStream _out( stdout, QIODevice::WriteOnly );

class PropertyHost : public WithProperties<PropertyHost> {};

class FooProperty : public SimpleDataProperty<PropertyHost, QString>
{
public:
	const char *name() const { return "foo"; }
} fooProperty;

void KopetePropertiesTest::allTests()
{
	PropertyHost myPropertyHost;
	CHECK( myPropertyHost.property(fooProperty).isNull(), true);
	myPropertyHost.setProperty( fooProperty, QString::fromLatin1("Foo!") );
	CHECK( myPropertyHost.property(fooProperty), QString::fromLatin1("Foo!") );
}
