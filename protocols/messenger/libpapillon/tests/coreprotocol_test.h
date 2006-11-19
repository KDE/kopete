/*
   coreprotocol_test.h - Test reconstitution of incoming packets.

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
#ifndef COREPROTOCOL_TEST_H
#define COREPROTOCOL_TEST_H

#include <QtCore/QObject>

class CoreProtocol_Test : public QObject
{
	Q_OBJECT
private slots:
	void testNormalTransfer();
	void testFullPayloadTransfer();
	void testFragmentPayloadTransfer();
};

#endif
