/*
   transfer_test.h - Unittest for Papillon::Transfer

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

#include <QtTest/QtTest>
#include <QtCore/QLatin1String>
#include <QtCore/QStringList>

#include "transfer_test.h"

#include <Papillon/Transfer>

using namespace Papillon;

void Transfer_Test::testNormalTransfer()
{
	Transfer normalTransfer;
	normalTransfer.setCommand("NLN");

	QStringList arguments;
	arguments << QLatin1String("NLN") << QLatin1String("test@passport.com") << QLatin1String("Test") << QLatin1String("0");

	normalTransfer.setArguments(arguments);

	QCOMPARE(normalTransfer.toString(), QString("NLN NLN test@passport.com Test 0\r\n"));
	QCOMPARE(normalTransfer.arguments().size(), 4);
}

void Transfer_Test::testTransactionTransfer()
{
	Transfer trTransfer(Transfer::TransactionTransfer);
	
	trTransfer.setCommand("USR");
	trTransfer.setTransactionId("1");
	
	QStringList arguments;
	arguments << QLatin1String("TWN") << QLatin1String("I") << QLatin1String("test@passport.com");
	trTransfer.setArguments(arguments);
	
	QCOMPARE(trTransfer.toString(), QString("USR 1 TWN I test@passport.com\r\n"));
	QCOMPARE(trTransfer.transactionId(), QString("1"));
}

void Transfer_Test::testPayloadTransfer()
{
	Transfer payTransfer(Transfer::PayloadTransfer | Transfer::TransactionTransfer);

	payTransfer.setCommand("UUX");
	payTransfer.setTransactionId("10");

	QByteArray data("<Data><PSM>Test</PSM><CurrentMedia></CurrentMedia></Data>");
	payTransfer.setPayloadData(data);

	QByteArray expectedRaw("UUX 10 57\r\n<Data><PSM>Test</PSM><CurrentMedia></CurrentMedia></Data>");

	QCOMPARE(payTransfer.payloadLength(), 57);
	QCOMPARE(payTransfer.toString(), QString("UUX 10 57\r\n"));
	QCOMPARE(payTransfer.toRawCommand(), expectedRaw);
}

void Transfer_Test::testStringArguments()
{
	Transfer transfer(Transfer::TransactionTransfer);
	
	transfer.setCommand("USR");
	transfer.setTransactionId("1");
	
	QString arguments("TWN I test@passport.com");
	transfer.setArguments(arguments);
	
	QCOMPARE(transfer.arguments()[0], QString("TWN"));
	QCOMPARE(transfer.arguments()[1], QString("I"));
	QCOMPARE(transfer.arguments()[2], QString("test@passport.com"));
}

QTEST_MAIN(Transfer_Test)

#include "transfer_test.moc"
