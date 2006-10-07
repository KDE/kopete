/*
   httptransfer_test.h - Unittest for HttpTransfer

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
#ifndef HTTPTRANSFER_TEST_H
#define HTTPTRANSFER_TEST_H

#include <QtCore/QObject>

class HttpTransfer_Test : public QObject
{
	Q_OBJECT
private slots:
	void testHttpRequest();
	void testHttpResponse();
};

#endif
