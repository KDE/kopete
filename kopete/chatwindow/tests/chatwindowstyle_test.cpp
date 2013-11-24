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

#include "chatwindowstyle_test.h"
#include <kunittest/module.h>

#include <stdlib.h>

#include <qdir.h>
#include <qfile.h>

#include <kopetechatwindowstyle.h>

KUNITTEST_MODULE( kunittest_chatwindowstyle_test, "KopeteChatWindowTestSuite");
KUNITTEST_MODULE_REGISTER_TESTER( ChatWindowStyle_Test );

void ChatWindowStyle_Test::allTests()
{
	testStyle = new ChatWindowStyle(QString(SRCDIR)+QString("/TestStyle"));

	// change user data dir to avoid messing with user's .kde dir
	setenv( "KDEHOME", QFile::encodeName( QDir::homePath() + "/.kopete-unittest" ), true );

	testPaths();
	testHtml();
	testVariants();
	testAction();
}

void ChatWindowStyle_Test::testPaths()
{
	QString expectedStylePath = SRCDIR + QString::fromUtf8("/TestStyle");
	QString expectedBaseHref = expectedStylePath + QString::fromUtf8("/Contents/Resources/");

	CHECK(testStyle->getStylePath(), expectedStylePath);
	CHECK(testStyle->getStyleBaseHref(), expectedBaseHref);
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

	CHECK(testStyle->getHeaderHtml(), exceptedHeader);
	CHECK(testStyle->getFooterHtml(), exceptedFooter);
	CHECK(testStyle->getIncomingHtml(), exceptedIncoming);
	CHECK(testStyle->getNextIncomingHtml(), exceptedNextIncoming);
	CHECK(testStyle->getOutgoingHtml(), exceptedOutgoing);
	CHECK(testStyle->getNextOutgoingHtml(), exceptedNextOutgoing);
	CHECK(testStyle->getStatusHtml(), exceptedStatus);
}

void ChatWindowStyle_Test::testAction()
{
	CHECK(testStyle->hasActionTemplate(), false);
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
		variantPathList.append(it.data());
	}	
	
	CHECK(variantNameList.join(";"), expectedNameResult);
	CHECK(variantPathList.join(";"), expectedPathResult);
}
