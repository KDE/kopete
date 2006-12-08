/*
   Unit test to test Windows Live Messenger Challenge.

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
#include "challengetest.h"

// Qt includes
#include <QtTest/QtTest>
#include <QtDebug>

// QCA includes
#include <QtCrypto>

// Papillon includes
// Use the #define hack to get acces to private methods in ChallengeTask
#define private public
#include "Papillon/Tasks/ChallengeTask"
#undef private

// This test the ChallengeHandler with Windows Live Messenger 8 product key/id
// Values from http://msnpiki.msnfanatic.com/index.php/MSNP13:Challenges
void Challenge_Test::testChallenge()
{
	// Convience object that init QCA.
	QCA::Initializer qcaInit;

	Papillon::ChallengeTask *challengeTask = new Papillon::ChallengeTask( new Papillon::Task(0, true) );
	
	QString resultChallenge1, resultChallenge2, resultChallenge3, resultChallenge4;
	resultChallenge1 = challengeTask->createChallengeHash( QString("15570131571988941333") );
	QCOMPARE( resultChallenge1, QString("47d2b1e24f85947bdd29b3bfe29f027b") );

	resultChallenge2 = challengeTask->createChallengeHash( QString("93662730714769834295") );
	QCOMPARE( resultChallenge2, QString("1fd1345b59d9e2c1af131e00b51ddb96") );

	resultChallenge3 = challengeTask->createChallengeHash( QString("13038318816579321232") );
	QCOMPARE( resultChallenge3, QString("f52409e13833d91f66af73abfced22bf") );

	resultChallenge4 = challengeTask->createChallengeHash( QString("23055170411503520698") );
	QCOMPARE( resultChallenge4, QString("780ee59b51acfd094658a55181fed7ca") );

	delete challengeTask;
}

QTEST_MAIN(Challenge_Test)

#include "challengetest.moc"
