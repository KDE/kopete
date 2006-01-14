/*
    Tests for Kopete::Message::parseEmoticons

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

#ifndef KOPETE_EMOTICON_TEST_H
#define KOPETE_EMOTICON_TEST_H

#include <kunittest/tester.h>

// change to SlotTester when it works
class KopeteEmoticonTest : public KUnitTest::Tester
{
public:
	//KopeteLinkTest();
	//~KopeteLinkTest();
	void allTests();
public slots:
	void testEmoticonParser();
private:
	
};

#endif


