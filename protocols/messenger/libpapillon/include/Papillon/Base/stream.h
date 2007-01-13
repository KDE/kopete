/*
   stream.h - Papillon Abstract class for a Stream(Notification/Switchboard)

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Based on code Copyright (c) 2004 Matt Rogers <mattr@kde.org>
   Based on code copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
   Based on Iris, Copyright (C) 2003 Justin Karneges

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLON_STREAM_H
#define PAPILLON_STREAM_H

#include <QtCore/QObject>

#include <Papillon/Macros>

namespace Papillon
{

class Transfer;

/**
 * @class Stream stream.h <Papillon/Base/Stream>
 */
class PAPILLON_EXPORT Stream : public QObject
{
	Q_OBJECT
public:
	enum Error { ErrParse, ErrProtocol, ErrStream, ErrCustom = 10 };
	enum StreamCond {
		GenericStreamError,
		Conflict,
		ConnectionTimeout,
		InternalServerError,
		InvalidFrom,
		PolicyViolation,
		ResourceConstraint,
		SystemShutdown
	};

	Stream(QObject *parent=0);
	virtual ~Stream();

	virtual void close()=0;
	virtual int errorCondition() const=0;
	virtual QString errorText() const=0;

	/**
	 * Are there any messages waiting to be read
	 */
	virtual bool transfersAvailable() const = 0;
	/**
	 * Read a message received from the server
	 */
	virtual Transfer *read() = 0;

	/**
	 * Send a message to the server
	 */
	virtual void write(Transfer *request) = 0;
	

signals:
	void connectionClosed();
	void delayedCloseFinished();
	void readyRead(); //signals that there is a transfer ready to be read
	void error(int);
};

}

#endif
