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

#include "chatwindowstyle_test.h"

#include <stdlib.h>

#include <qdir.h>
#include <qfile.h>
#include <kdebug.h>

#include <kopetechatwindowstyle.h>

void ChatWindowStyle_Test::initTestCase()
{
	testStyle = new ChatWindowStyle(QString(KDESRCDIR)+QString("TestStyle"));

  // change user data dir to avoid messing with user's .kde dir
	setenv( "KDEHOME", QFile::encodeName( QDir::homePath() + "/.kopete-unittest" ), true );

}
void ChatWindowStyle_Test::cleanupTestCase()
{
}

void ChatWindowStyle_Test::testPaths()
{
	QString expectedStylePath = KDESRCDIR + QString::fromUtf8("TestStyle");
	QString expectedBaseHref = expectedStylePath + QString::fromUtf8("/Contents/Resources/");

	QCOMPARE(testStyle->getStyleName(), expectedStylePath);
	QCOMPARE(testStyle->getStyleBaseHref(), expectedBaseHref);
}

void ChatWindowStyle_Test::testHtml()
{
	QString exceptedHeader = QString::fromUtf8(
"<div>%chatName%</div>\n"
"<div>%sourceName%</div>\n"
"<div>%destinationName%</div>\n"
"<div>%incomingIconPath%</div>\n"
"<div>%outgoingIconPath%</div>\n"
"<div>%timeOpened%</div>\n"
"<div>%timeOpened{%H:%M}%</div>");
	// Footer is empty on purpose, this is to test if the file doesn't exist.
	QString exceptedFooter;
	QString exceptedIncoming = QString::fromUtf8(
"Incoming:\n"
"<div>%userIconPath%</div>\n"
"<div>%senderScreenName%</div>\n"
"<div>%sender%</div>\n"
"<div>%service%</div>\n"
"<div>%message%</div>\n"
"<div>%time%</div>\n"
"<div>%time{%H:%M}%</div>\n"
"<div id=\"insert\">");
	QString exceptedNextIncoming = QString::fromUtf8(
"Incoming:\n"
"<div>%message%</div>\n"
"<div id=\"insert\">"
);
	QString exceptedOutgoing = QString::fromUtf8(
"Outgoing:\n"
"<div>%userIconPath%</div>\n"
"<div>%senderScreenName%</div>\n"
"<div>%sender%</div>\n"
"<div>%service%</div>\n"
"<div>%message%</div>\n"
"<div>%time%</div>\n"
"<div>%time{%H:%M}%</div>\n"
"<div id=\"insert\">");
	QString exceptedNextOutgoing = QString::fromUtf8(
"Outgoing:\n"
"<div>%message%</div>\n"
"<div id=\"insert\">"
);
	QString exceptedStatus = QString::fromUtf8(
"<div>%message%</div>\n"
"<div>%time%</div>\n"
"<div>%time{%H:%M}%</div>");

	QCOMPARE(testStyle->getHeaderHtml(), exceptedHeader);
	QCOMPARE(testStyle->getFooterHtml(), exceptedFooter);
	QCOMPARE(testStyle->getIncomingHtml(), exceptedIncoming);
	QCOMPARE(testStyle->getNextIncomingHtml(), exceptedNextIncoming);
	QCOMPARE(testStyle->getOutgoingHtml(), exceptedOutgoing);
	QCOMPARE(testStyle->getNextOutgoingHtml(), exceptedNextOutgoing);
	QCOMPARE(testStyle->getStatusHtml(), exceptedStatus);
}

void ChatWindowStyle_Test::testAction()
{
	QCOMPARE(testStyle->hasActionTemplate(), false);
}

void ChatWindowStyle_Test::testVariants()
{
	QString expectedNameResult("Variant1;Variant2");
	QString expectedPathResult("Variants/Variant1.css;Variants/Variant2.css");
	QStringList variantNameList;
	QStringList variantPathList;
	ChatWindowStyle::StyleVariants variantList;
	ChatWindowStyle::StyleVariants::ConstIterator it;
	variantList = testStyle->getVariants();

	for(it = variantList.constBegin(); it != variantList.constEnd(); ++it)
	{
		variantNameList.append(it.key());
		variantPathList.append(it.value());
	}	
	
	QCOMPARE(variantNameList.join(";"), expectedNameResult);
	QCOMPARE(variantPathList.join(";"), expectedPathResult);
}

QTEST_MAIN(ChatWindowStyle_Test)
#include "chatwindowstyle_test.moc"