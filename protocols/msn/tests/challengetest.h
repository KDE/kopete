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
#ifndef CHALLENGETEST_H
#define CHALLENGETEST_H

#include <QObject>

class ChallengeTest : public QObject
{
	Q_OBJECT
private slots:
	void testChallenge();
	void testChallengeMsn13();
};

#endif 
