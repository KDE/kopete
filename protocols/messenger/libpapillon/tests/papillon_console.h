/*
   papillon_console.h - GUI Papillon debug console.

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
#ifndef PAPILLON_CONSOLE_H
#define PAPILLON_CONSOLE_H

#include <QtGui/QWidget>

// Little hack to access writeCommand method in Client
#define private public
#include "Papillon/Client"
#undef private

namespace Papillon
{
	class FetchContactListJob;
}

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
	void buttonTestContactList();

	void clientConnectionStatusChanged(Papillon::Client::ConnectionStatus status);
	void contactListLoaded();

private:
	bool isPayloadCommand(const QString &command);

	class Private;
	Private *d;
};

}
#endif
