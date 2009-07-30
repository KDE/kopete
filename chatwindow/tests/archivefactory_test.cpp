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

#include "archivefactory_test.h"
#include <karchive.h>
#include <kzip.h>
#include <ktar.h>
#include "kopetechatwindowstylearchivefactory.h"

void ArchiveFactory_Test::initTestCase()
{
}

void ArchiveFactory_Test::cleanupTestCase()
{
}

void ArchiveFactory_Test::testTar()
{
	KArchive* archive = KopeteChatWindowStyleArchiveFactory::getKArchive(KDESRCDIR+QString("TestArchives/test.tar.gz"));

	delete archive;
}
void ArchiveFactory_Test::testZip()
{
	KArchive* archive = KopeteChatWindowStyleArchiveFactory::getKArchive(KDESRCDIR+QString("TestArchives/test.zip"));
	delete archive;
}
void ArchiveFactory_Test::testBadArchive()
{
	KArchive* archive = KopeteChatWindowStyleArchiveFactory::getKArchive(KDESRCDIR+QString("TestArchives/empty.txt"));
	QCOMPARE(archive,NULL);
	delete archive;
}


QTEST_MAIN(ArchiveFactory_Test)
#include "archivefactory_test.moc"

