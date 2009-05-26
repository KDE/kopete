/*
    Tests for Kopete::ContactList class.

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

#ifndef KOPETECONTACTLIST_TEST_H
#define KOPETECONTACTLIST_TEST_H

#include <kunittest/tester.h>

// change to SlotTester when it works
class KopeteContactList_Test : public KUnitTest::Tester
{
public:
	void allTests();
public slots:
	void testSomething();
private:
	
};

#endif

