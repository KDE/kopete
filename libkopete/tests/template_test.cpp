/*
    Tests for some requirement

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

#include "template_test.h"
#include <kunittest/module.h>

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_template_test, "KopeteSuite");
KUNITTEST_MODULE_REGISTER_TESTER( Template_Test );

void Template_Test::allTests()
{
	testSomething();
}

void Template_Test::testSomething()
{
		int result = 1;
		int expected = 1;
		// result should be the expected one
		CHECK(result, expected);
}
