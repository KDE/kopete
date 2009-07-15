/*
    Test RichTextBox Component


	Copyright (c) 2009      by Pierre-Alexandre St-Jean <pierrealexandre.stjean@gmail.com>

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
#ifndef ARCHIVEFACTORY_TEST_H
#define ARCHIVEFACTORY_TEST_H

#include <QtTest>



class ArchiveFactory_Test : public QObject
{

	Q_OBJECT;

private slots:
	void initTestCase();
	void cleanupTestCase();

	void testTar();
	void testZip();
	void testBadArchive();

private:

};

#endif
