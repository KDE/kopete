/*
    ChatWindowStyle test suite

    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>
    Copyright (c) 2009      by Pierre-Alexandre St-Jean       <pierrealexandre.stjean@gmail.com>

    Kopete    (c) 2002-2009 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef CHATWINDOWSTYLE_TEST_H
#define CHATWINDOWSTYLE_TEST_H

#include <QtTest>

class ChatWindowStyle;

class ChatWindowStyle_Test : public QObject
{
		Q_OBJECT;

private slots:
	void initTestCase();
	void cleanupTestCase();

	void testPaths();
	void testHtml();
	void testVariants();
	void testAction();
	
private:
	ChatWindowStyle *testStyle;
};

#endif
