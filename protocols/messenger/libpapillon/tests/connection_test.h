/*
   connection_test.h - Connection test

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
#ifndef CONNECTIONTEST_H
#define CONNECTIONTEST_H

#include <QtCore/QObject>
#include <Papillon/Client>

using namespace Papillon;

class Connection_Test : public QObject
{
	Q_OBJECT
public slots:
	void testConnection();

private slots:
	void clientConnectionStatusChanged(Papillon::Client::ConnectionStatus status);

private:
	Client *m_client;
};


#endif
