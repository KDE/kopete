/*
   httptransfer_test.h - Unittest for HttpTransfer

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "httptransfer_test.h"

// Qt includes
#include <QtTest/QtTest>
#include <QtCore/QLatin1String>

// Papillon includes
#include "httptransfer.h"

using namespace Papillon;

void HttpTransfer_Test::testHttpRequest()
{
	HttpTransfer request;
	QByteArray testBody = QByteArray("<html>\n<head></head>\n<body></body>\n</html>");
	QCOMPARE((int)request.type(), (int)HttpTransfer::HttpRequest);
	
	request.setRequest( QLatin1String("GET"), QLatin1String("/abservice/abservice.asmx") );
	
	QCOMPARE( request.method(), QString("GET") );
	QCOMPARE( request.path() , QString("/abservice/abservice.asmx") );
	
	request.setContentType( QLatin1String("text/html") );
	request.setBody(testBody);

	QCOMPARE( request.contentType(), QString("text/html") );
	QCOMPARE( request.contentLength(), (uint)testBody.size() );
}

void HttpTransfer_Test::testHttpResponse()
{
	
}


QTEST_MAIN(HttpTransfer_Test);

#include "httptransfer_test.moc"
