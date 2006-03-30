/*
   papillon_console.h - GUI Papillon debug console.

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
#ifndef PAPILLON_CONSOLE_H
#define PAPILLON_CONSOLE_H

#include <QtGui/QWidget>

namespace PapillonConsole
{

void guiDebugOutput(QtMsgType type, const char *msg);

class PapillonConsole : public QWidget
{
	friend void guiDebugOutput(QtMsgType type, const char *msg);

	Q_OBJECT
public:
	PapillonConsole(QWidget *parent = 0);
	~PapillonConsole();

private slots:
	void buttonSendClicked();
	void buttonConnectClicked();
	void buttonTestSOAP();
	void clientConnected();
	void streamConnected();
	void streamReadyRead();

private:
	bool isPayloadCommand(const QString &command);

	class Private;
	Private *d;
};

}
#endif
