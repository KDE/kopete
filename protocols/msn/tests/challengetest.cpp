/*
    Unit test to test MSN Challenge.

    Copyright (c) 2006      by Michaël Larouche       <michael.larouche@kdemail.net>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qtest_kde.h>
#include "challengetest.h"
#include "challengetest.moc"

QTEST_KDEMAIN( ChallengeTest, GUI )

#include "msnchallengehandler.h"

// This test the ChallengeHandler with MSN Messenger 7.0.0813 product key/id
// Values from http://msnpiki.msnfanatic.com/index.php/MSNP11:Challenges
void ChallengeTest::testChallenge()
{
	MSNChallengeHandler *challengeHandler = new MSNChallengeHandler("CFHUR$52U_{VIX5T", "PROD0101{0RM?UBW");
	
	QString resultChallenge1, resultChallenge2, resultChallenge3, resultChallenge4;
	resultChallenge1 = challengeHandler->computeHash( QString("13038318816579321232") );
	QCOMPARE( resultChallenge1, QString("5f6ae998b8fa70c37b62980f71a0e736") );

	resultChallenge2 = challengeHandler->computeHash( QString("23055170411503520698") );
	QCOMPARE( resultChallenge2, QString("934ab429609709f4fe70e5fa930993c2") );

	resultChallenge3 = challengeHandler->computeHash( QString("37819769320541083311") );
	QCOMPARE( resultChallenge3, QString("40575bf9740af7cad8671211e417d0cb") );

	resultChallenge4 = challengeHandler->computeHash( QString("93662730714769834295") );
	QCOMPARE( resultChallenge4, QString("003ed1b1be3ca0d81f83a587ebbe3675") );

	delete challengeHandler;
}

// This test the ChallengeHandler with Windows Live Messenger 8 product key/id
// Values from http://msnpiki.msnfanatic.com/index.php/MSNP13:Challenges
void ChallengeTest::testChallengeMsn13()
{
	MSNChallengeHandler *challengeHandler = new MSNChallengeHandler("O4BG@C7BWLYQX?5G", "PROD01065C%ZFN6F");
	
	QString resultChallenge1, resultChallenge2, resultChallenge3, resultChallenge4;
	resultChallenge1 = challengeHandler->computeHash( QString("15570131571988941333") );
	QCOMPARE( resultChallenge1, QString("47d2b1e24f85947bdd29b3bfe29f027b") );

	resultChallenge2 = challengeHandler->computeHash( QString("93662730714769834295") );
	QCOMPARE( resultChallenge2, QString("1fd1345b59d9e2c1af131e00b51ddb96") );

	resultChallenge3 = challengeHandler->computeHash( QString("13038318816579321232") );
	QCOMPARE( resultChallenge3, QString("f52409e13833d91f66af73abfced22bf") );

	resultChallenge4 = challengeHandler->computeHash( QString("23055170411503520698") );
	QCOMPARE( resultChallenge4, QString("780ee59b51acfd094658a55181fed7ca") );

	delete challengeHandler;
}
