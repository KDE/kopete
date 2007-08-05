/*
   transfer_test.h - Unittest for Papillon::NetworkMessage

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

#include "networkmessage_test.h"

#include <Papillon/NetworkMessage>

using namespace Papillon;

void NetworkMessage_Test::testNormalMessage()
{
	NetworkMessage normalMessage;
	normalMessage.setCommand("NLN");

	QStringList arguments;
	arguments << QLatin1String("NLN") << QLatin1String("test@passport.com") << QLatin1String("Test") << QLatin1String("0");

	normalMessage.setArguments(arguments);

	QCOMPARE(normalMessage.toString(), QString("NLN NLN test@passport.com Test 0\r\n"));
	QCOMPARE(normalMessage.arguments().size(), 4);
}

void NetworkMessage_Test::testTransactionMessage()
{
	NetworkMessage trMessage(NetworkMessage::TransactionMessage);
	
	trMessage.setCommand("USR");
	trMessage.setTransactionId("1");
	
	QStringList arguments;
	arguments << QLatin1String("TWN") << QLatin1String("I") << QLatin1String("test@passport.com");
	trMessage.setArguments(arguments);
	
	QCOMPARE(trMessage.toString(), QString("USR 1 TWN I test@passport.com\r\n"));
	QCOMPARE(trMessage.transactionId(), QString("1"));
}

void NetworkMessage_Test::testPayloadMessage()
{
	NetworkMessage payMessage( NetworkMessage::PayloadMessage | NetworkMessage::TransactionMessage );

	payMessage.setCommand("UUX");
	payMessage.setTransactionId("10");

	QByteArray data("<Data><PSM>Test</PSM><CurrentMedia></CurrentMedia></Data>");
	payMessage.setPayloadData(data);

	QByteArray expectedRaw("UUX 10 57\r\n<Data><PSM>Test</PSM><CurrentMedia></CurrentMedia></Data>");

	QCOMPARE(payMessage.payloadLength(), 57);
	QCOMPARE(payMessage.toString(), QString("UUX 10 57\r\n"));
	QCOMPARE(payMessage.toRawCommand(), expectedRaw);
}

void NetworkMessage_Test::testStringArguments()
{
	NetworkMessage message( NetworkMessage::TransactionMessage );
	
	message.setCommand("USR");
	message.setTransactionId("1");
	
	QString arguments("TWN I test@passport.com");
	message.setArguments(arguments);
	
	QCOMPARE(message.arguments()[0], QString("TWN"));
	QCOMPARE(message.arguments()[1], QString("I"));
	QCOMPARE(message.arguments()[2], QString("test@passport.com"));
}

QTEST_MAIN(NetworkMessage_Test)

#include "networkmessage_test.moc"
