/*
   connector_test.h - Test for the connection architecture.

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef CONNECTOR_TEST_H
#define CONNECTOR_TEST_H

#include <QObject>

class Connector_Test : public QObject
{
	Q_OBJECT
public:
	Connector_Test(QObject *parent = 0);
	~Connector_Test();

public slots:
	void connectToServer();

private slots:
	void slotReadTransfer();
	void slotConnected();

	void doLoginProcess();
	void loginProcessCvr();
	void loginProcessTwnI();
	void loginProcessTwnS();
	
	void slotExit();

private:
	class Private;
	Private *d;
};
#endif
