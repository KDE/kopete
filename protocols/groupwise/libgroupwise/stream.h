/*
    stream.h - Kopete Groupwise Protocol
    
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qdom.h>
#include "qobject.h"

#include "gwerror.h"
#include "gwfield.h"
#include "request.h"

#ifndef GW_STREAM_H
#define GW_STREAM_H


class Stream : public QObject
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
/*#		InvalidXml,  // not required*/
		PolicyViolation,
		ResourceConstraint,
		SystemShutdown
	};

	Stream(QObject *parent=0);
	virtual ~Stream();

	virtual void close()=0;
	virtual int errorCondition() const=0;
	virtual QString errorText() const=0;
	//virtual QDomElement errorAppSpec() const=0;

	/**
	 * Are there any messages waiting to be read
	 */
	virtual bool transfersAvailable() const = 0;	// adapt to messages
	/**
	 * Read a message received from the server
	 */
	virtual Transfer * read() = 0;

	/**
	 * Send a message to the server
	 */
	virtual void write( Request *request) = 0;	// ", ends up on a send queue, by a very roundabout way, see analysis at bottom of 
	
// #	virtual bool stanzaAvailable() const=0;
// #	virtual Stanza read()=0;
// #	virtual void write(const Stanza &s)=0;

// #	virtual QDomDocument & doc() const=0;
// #	virtual QString baseNS() const=0;
// #	virtual bool old() const=0;

// #	Stanza createStanza(Stanza::Kind k, const Jid &to="", const QString &type="", const QString &id="");
// #	Stanza createStanza(const QDomElement &e);

//	static QString xmlToString(const static XmlProtocol *foo = 0;
//QDomElement &e, bool clip=false);

signals:
	void connectionClosed();
	void delayedCloseFinished();
	void readyRead();
//	void stanzaWritten();
	void error(int);
};

#endif
