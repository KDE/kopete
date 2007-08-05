/*
   coreprotocol_test.cpp - Test reconstitution of incoming packets.

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
#include "coreprotocol_test.h"

// Qt includes
#include <QtTest/QtTest>
#include <QtCore/QFile>
#include <QtCore/QLatin1String>
#include <QtDebug>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/MessengerCoreProtocol"

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

void CoreProtocol_Test::testNormalNetworkMessage()
{
	MessengerCoreProtocol protocol;
	QByteArray data = readDataFromFile( QLatin1String(PAPILLON_TESTS_DATA"wlm_transfer1.transfer") );
	protocol.addIncomingData(data);
	
	NetworkMessage *networkMessage = protocol.incomingNetworkMessage();

	QVERIFY( networkMessage != 0 );
	QVERIFY( networkMessage->type() & NetworkMessage::TransactionMessage );
	QCOMPARE( networkMessage->command(), QString("VER") );
	QCOMPARE( networkMessage->transactionId(), QString("1") );
	QCOMPARE( networkMessage->arguments().count(), 2 );
	QCOMPARE( networkMessage->arguments()[0], QString("MSNP13") );
	QCOMPARE( networkMessage->arguments()[1], QString("CVR0") );
}

void CoreProtocol_Test::testFullPayloadNetworkMessage()
{
	MessengerCoreProtocol protocol;
	QByteArray data = readDataFromFile( QLatin1String(PAPILLON_TESTS_DATA"wlm_transfer2.transfer") );

	protocol.addIncomingData(data);

	NetworkMessage *networkMessage = protocol.incomingNetworkMessage();

	QVERIFY( networkMessage != 0 );
	QVERIFY( networkMessage->type() & NetworkMessage::PayloadMessage );
	QCOMPARE( networkMessage->command(), QString("GCF") );
	QCOMPARE( networkMessage->payloadLength(), 165 );
	QCOMPARE( networkMessage->payloadData().size(), 165 );

	//delete networkMessage;
}

void CoreProtocol_Test::testFragmentPayloadNetworkMessage()
{
	MessengerCoreProtocol protocol;
	QByteArray data1 = readDataFromFile( QLatin1String(PAPILLON_TESTS_DATA"wlm_transfer3.transfer") );
	QByteArray data2 = readDataFromFile( QLatin1String(PAPILLON_TESTS_DATA"wlm_transfer4.transfer") );
	
	protocol.addIncomingData(data1);

	QVERIFY( protocol.incomingNetworkMessage() == 0 );

	protocol.addIncomingData(data2);

	NetworkMessage *networkMessage = protocol.incomingNetworkMessage();

	QVERIFY( networkMessage != 0 );
	QVERIFY( networkMessage->type() & NetworkMessage::PayloadMessage );
	QCOMPARE( networkMessage->command(), QString("MSG") );
	QCOMPARE( networkMessage->payloadLength(), 553 );

	QByteArray expectedData = QByteArray("MSG Hotmail Hotmail 553\r\n"
"MIME-Version: 1.0\r\n"
"Content-Type: text/x-msmsgsprofile; charset=UTF-8\r\n"
"LoginTime: 1139359068\r\n"
"EmailEnabled: 0\r\n"
"MemberIdHigh: 196608\r\n"
"MemberIdLow: -2097247763\r\n"
"lang_preference: 1036\r\n"
"preferredEmail: \r\n"
"country: CA\r\n"
"PostalCode: \r\n"
"Gender: \r\n"
"Kid: 0\r\n"
"Age: \r\n"
"BDayPre: \r\n"
"Birthday: \r\n"
"Wallet: \r\n"
"Flags: 1073759809\r\n"
"sid: 507\r\n"
"kv: 7\r\n"
"MSPAuth: 7Upxp5g1eAv*vBoWdtRVE2gk9*4uM!gUEeW*9Ab6d12Ghfq18yqPWeJq70NcRVEakkzPG7yZOp1YZLVw8bpbPy4ydnEP0vwf6ihNdQT4jdXysqHtKoAYT2sKhtccbgCzqrPpinKXEqDzk$\r\n"
"ClientIP: 142.169.84.237\r\n"
"ClientPort: 47744\r\n"
"ABCHMigrated: 1\r\n"
"BetaInvites: 0\r\n\r\n");

	QCOMPARE( networkMessage->toRawCommand(), expectedData );
}

QTEST_MAIN(CoreProtocol_Test)

#include "coreprotocol_test.moc"
