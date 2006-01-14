/*
    messageformatter.h - msn p2p protocol

    Copyright (c) 2005      by Gregg Edghill          <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MESSAGEFORMATTER_H
#define MESSAGEFORMATTER_H

#include <qobject.h>

namespace P2P{
	class Message;
}

/**
@author Kopete Developers
*/
namespace P2P{
	class MessageFormatter : public QObject
	{	Q_OBJECT
		public:
			MessageFormatter(QObject *parent = 0, const char *name = 0);
			~MessageFormatter();

			Message readMessage(const QByteArray& stream, bool compact=false);
			void writeMessage(const Message& message, QByteArray& stream, bool compact=false);
	};
}

#endif
