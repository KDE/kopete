/*
    Adium(and Kopete) ChatWindowStyle format rendering test suite

    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>
	Copyright (c) 2009      by Pierre-Alexandre St-Jean <pierrealexandre.stjean@gmail.com>

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

#ifndef CHATWINDOWSTYLERENDERING_TEST_H
#define CHATWINDOWSTYLERENDERING_TEST_H

#include <QtTest>

// HACK: Needed to access private methods of ChatMessagePart.
#define private public
#include <chatmessagepart.h>
#undef private

class ChatWindowStyleRendering_Test : public QtTest
{

	Q_OBJECT;

private slots:
	void initTestCase();
	void testHeaderRendering();
	void testMessageRendering();
	void testStatusRendering();
	void testFullRendering();
	void cleanupTestCase();

private:
	class Private;
	Private * const d;

}
#endif
