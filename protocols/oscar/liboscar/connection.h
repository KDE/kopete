/*
Kopete Oscar Protocol
connection.h - independent protocol encapsulation

Copyright (c) 2004-2005 by Matt Rogers <mattr@kde.org>

Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

*************************************************************************
*                                                                       *
* This library is free software; you can redistribute it and/or         *
* modify it under the terms of the GNU Lesser General Public            *
* License as published by the Free Software Foundation; either          *
* version 2 of the License, or (at your option) any later version.      *
*                                                                       *
*************************************************************************
*/
#ifndef CONNECTION_H
#define CONNECTION_H

#include <qobject.h>
#include <qvaluelist.h>
#include "oscartypes.h"
#include "rateclass.h"

class ConnectionPrivate;
class Client;
class ClientStream;
class Connector;
class ByteStream;
class Transfer;
class RateClassManager;
class SSIManager;
class Task;


namespace Oscar
{
class Settings;
}

/**
 * This class encapsulates both the low level network layer code and the high
 * level OSCAR protocol code required to create a single independent
 * connection to an OSCAR server
 * @author Matt Rogers
 */
class Connection : public QObject
{
Q_OBJECT
public:

	Connection( Connector* connector, ClientStream* cs, const char* name = 0 );
	~Connection();

	void setClient( Client* );

	void connectToServer( const QString& server, bool auth = true );
	/**
	 * Close the connection and reset the connection data
	 */
	void close();

	/**
	 * Check to see if the family specified by @p family is supported by this
	 * connection.
	 * @param family the family number to check
	 */
	bool isSupported( int family ) const;

	/**
	 * Get the list of supported families
	 * @return The list of families supported on this connection
	 */
	QValueList<int> supportedFamilies() const;

	/**
	 * Add the SNAC families in \p familyList to the list of supported families for
	 * this connection
	 * \param familyList the list of families to add
	 */
	void addToSupportedFamilies( const QValueList<int>& familyList );

	/**
	 * Add the SNAC family in \p family to the list of supported families for
	 * this connection
	 * \overload
	 * \param family the single family to add to the list
	 */
	void addToSupportedFamilies( int family );

	/**
	 * Add the rate classes in \p rateClassList to the list of rate classes packets
	 * need to be filtered on
	 * \param rateClassList the list of rate classes to add
	 */
	void addToRateClasses( const QValueList<RateClass*> rateClassList );

	/**
	 * Add the rate class in \p rc to the list of rate classes packets
	 * need to be filtered on
	 * \overload
	 * \param rc the list rate class to add
	 */
	void addToRateClasses( RateClass* rc );

	/**
	 * Indicate to the connection that there has been an error in a task. The
	 * error won't require us to go offline, but the user should be notified
	 * about the error
	 * \param s the SNAC the error occured from
	 * \param errCode the error code
	 */
	void taskError( const Oscar::SNAC& s, int errCode );

	/**
	 * Indicate to the connection that there has been a fatal error in a task.
	 * This error will require a disconnection from the OSCAR service and if
	 * necessary, the user should be prompted to reconnect manually or an
	 * automatic reconnection should be attempted.
	 * \param s the SNAC the error occured from
	 * \param errCode the error code
	 */
	void fatalTaskError( const Oscar::SNAC& s, int errCode );

    /**
     * Get the chat room name for this connection.
     * @return the name of the room or QString::null if not connected to a room
     */

	/** Get the user settings object */
	Oscar::Settings* settings() const;

	/** Get the current FLAP sequence for this connection */
	Q_UINT16 flapSequence();

	/** Get the current SNAC sequence for this connection */
	Q_UINT32 snacSequence();

	/** Get the cookie for this connection */
	QByteArray cookie() const;

	QString userId() const;
	QString password() const;
	bool isIcq() const;
	SSIManager* ssiManager() const;
	const Oscar::ClientVersion* version() const;
	RateClassManager* rateManager() const;
	bool isLoggedIn() const;

	/** Convenience function to get the root task for use in Tasks */
	Task* rootTask() const;

	/** Get the raw connector for this connection, in case we need it */
	Connector* connector();

	/** Get the byte stream for this connection, in case we need it */
	ByteStream* byteStream();

	void send( Transfer* t ) const;
	void forcedSend( Transfer* t ) const;

signals:

	/** There's data ready to read */
	void readyRead();

	/** We've connected */
	void connected();

	/** We were disconnected */
	void disconnected();

	/**
	 * There was an error on the socket and we've disconnected
	 * \param errCode the error code from the operating system
	 * \param errString the i18n'ed string that describes the error
	 */
	void socketError( int errCode, const QString& errString );


private:
	/** Seed the sequence numbers with random values */
	void initSequence();

	/** Distribute the transfer among the connection's tasks */
	void distribute( Transfer* t ) const;

private slots:
	/** Reset the data for the connection.*/
	void reset();

	/** We've got something from the stream */
	void streamReadyRead();

	/** We've finished logging in */
	void loggedIn();

	void streamSocketError( int );

private:

	ConnectionPrivate* d;
	bool m_loggedIn;
};

#endif

//kate: tab-width 4; indent-mode csands; auto-insert-doxygen on;
