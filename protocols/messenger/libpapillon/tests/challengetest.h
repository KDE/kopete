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
#ifndef CHALLENGETEST_H
#define CHALLENGETEST_H

#include <QtCore/QObject>

class Challenge_Test : public QObject
{
	Q_OBJECT
private slots:
	void testChallenge();
};

#endif 
