/*
    test1.cpp

    Copyright (c) 2007      by Charles Connell        <charles@connells.org>

    Kopete    (c) 2007      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "tests.h"

#include <QDomDocument>

QString KopeteXmlParserTests::test1 ( const QString & str ) {
	
	QDomDocument doc;
	doc.setContent ( str );
	
	QDomElement body = doc.firstChildElement ( "message" ).firstChildElement ( "body" );
	body.setAttribute ( "color", "red" );
	body.firstChild().toText().setData ( "You are not a special or unique snowflake." );
	
	return doc.toString();
}


