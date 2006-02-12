/*
   connection_test.h - Test for the connection architecture.

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
#ifndef CONNECTION_TEST_H
#define CONNECTION_TEST_H

#include <QObject>

class Connection_Test : public QObject
{
	Q_OBJECT
public:
	Connection_Test(QObject *parent = 0);
	~Connection_Test();

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
