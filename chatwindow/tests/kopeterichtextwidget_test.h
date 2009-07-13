/*
    ChatWindowStyle test suite

    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>

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
#ifndef KOPETERICHTEXTWIDGET_TEST_H
#define KOPETERICHTEXTWIDGET_TEST_H

#include <QtGui>
#include <QtTest>

#include "kopeterichtextwidget.h"

class KopeteRichTextWidget_Test : public QObject
{

	Q_OBJECT;

private slots:
	void initTestCase();
	void cleanupTestCase();

	void testIt();

private:

};

#endif
