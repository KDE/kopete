/*
   httpcoreprotocol_test.h - Test reconstitution of incoming packets for HTTP

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
#ifndef HTTPCOREPROTOCOL_TEST_H
#define HTTPCOREPROTOCOL_TEST_H

#include <QtCore/QObject>

class HttpCoreProtocol_Test : public QObject
{
	Q_OBJECT
private slots:
	void testNormalTransfer();
};

#endif
