/*
   httpcoreprotocol_test.cpp - Test reconstitution of incoming packets for HTTP

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "httpcoreprotocol_test.h"

// Qt includes
#include <QtTest/QtTest>
#include <QtCore/QFile>
#include <QtCore/QLatin1String>

// Papillon includes
#include "Papillon/Http/Transfer"
#include "Papillon/Http/CoreProtocol"

#ifndef PAPILLON_TESTS_DATA
#define PAPILLON_TESTS_DATA
#endif

using namespace Papillon;

QByteArray readDataFromFile(const QString &fileName)
{
	QFile dataFile(fileName);
	dataFile.open(QIODevice::ReadOnly);
	
	QByteArray temp = dataFile.readAll();

	dataFile.close();

	return temp;
}

void HttpCoreProtocol_Test::testNormalTransfer()
{
	HttpCoreProtocol protocol;
	QByteArray data = readDataFromFile( QLatin1String(PAPILLON_TESTS_DATA"http_transfer1.transfer") );
	protocol.addIncomingData(data);
	
	HttpTransfer *transfer = protocol.incomingTransfer();

	QVERIFY( transfer != 0 );
	QVERIFY( transfer->type() == HttpTransfer::HttpResponse );
	QVERIFY( transfer->hasContentLength() );
	QVERIFY( transfer->hasContentType() );
	QCOMPARE( transfer->statusCode(), 200 );
	QCOMPARE( transfer->contentType(), QString("application/xml") );
	QCOMPARE( transfer->contentLength(), (uint)4171 );
	QCOMPARE( (uint)transfer->body().size(), transfer->contentLength() );
	int indexOf = transfer->body().lastIndexOf("</soap:Envelope>");
	QVERIFY( indexOf != 1 );
}

QTEST_MAIN(HttpCoreProtocol_Test)

#include "httpcoreprotocol_test.moc"
