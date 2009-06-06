/*
    Kopete Groupwise Protocol
    inputprotocolbase.h - Ancestor of all protocols used for reading GroupWise input

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
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

#ifndef INPUTPROTOCOLBASE_H
#define INPUTPROTOCOLBASE_H

#include <QObject>
#include <QByteArray>
#include <QDataStream>

class Transfer;
/**
Defines a basic interface for protocols dealing with input from the GroupWise server.

@author Kopete Developers
*/
class InputProtocolBase : public QObject
{
Q_OBJECT
public:
	enum EventProtocolState { Success, NeedMore, OutOfSync, ProtocolError };
	InputProtocolBase(QObject *parent = 0);
	~InputProtocolBase();

	/**
	 * Debug output
	 */
	static void debug(const QString &str);

	/**
	 * Returns a value describing the state of the object.  
	 * If the object is given data to parse that does not begin with a recognised event code, 
	 * it will become OutOfSync, to indicate that the input data probably contains leftover data not processed during a previous parse.
	 */
	uint state() const;
	/**
	 * Attempt to parse the supplied data into a Transfer object
	 * @param bytes this will be set to the number of bytes that were successfully parsed.  It is no indication of the success of the whole procedure
	 * @return On success, a Transfer object that the caller is responsible for deleting.  It will be either an EventTransfer or a Response, delete as appropriate.  On failure, returns 0.
	 */
	virtual Transfer * parse( QByteArray &, uint & bytes ) = 0 ;
protected:
	/**
	 * Reads an arbitrary string
	 * updates the bytes parsed counter
	 */
	bool readString( QString &message );
	/**
	 * Check that there is data to read, and set the protocol's state if there isn't any.
	 */
	bool okToProceed();
	/** 
	 * read a quint32 giving the number of following bytes, then a string of that length
	 * updates the bytes parsed counter
	 * @return false if the string was broken or there was no data available at all
	 */
	bool safeReadBytes( QByteArray & data, uint & len );
	
protected:
	uint m_state;
	uint m_bytes;
	QDataStream m_din;
};

#endif
