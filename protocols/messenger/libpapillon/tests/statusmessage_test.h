/*
   statusmessage_test.h - Unittest for Papillon::StatusMessage

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
#ifndef STATUSMESSAGE_TEST_H
#define STATUSMESSAGE_TEST_H

#include <QObject>

class StatusMessage_Test : public QObject
{
	Q_OBJECT
private slots:
	void testStatusMessageParsing();
	void testStatusMessageGeneration();
};
#endif
