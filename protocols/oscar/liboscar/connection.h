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
#include <QList>
#include "oscartypes.h"
#include "rateclass.h"
#include "liboscar_export.h"

class ConnectionPrivate;
class ClientStream;
class Transfer;
class RateClassManager;
class ContactManager;
class Task;
class QHostAddress;


namespace Oscar
{
class Client;
class Settings;

class MessageInfo
{
public:
	MessageInfo() : id( 0 ) {}
	bool isValid() const { return (id > 0 && !contact.isEmpty()); }

	uint id;
	QString contact;
};

}

/**
 * @brief a single independent connection to an OSCAR server
 * 
 * This class encapsulates both the low level network layer code and the high
 * level OSCAR protocol code required to create a single independent
 * connection to an OSCAR server
 * @author Matt Rogers
 */
class LIBOSCAR_EXPORT Connection : public QObject
{
Q_OBJECT
public:

	explicit Connection( ClientStream* cs, const char* name = 0 );
	~Connection();

	static void setStartFlapSequenceList( const QList<Oscar::WORD>& seqList );
	
	void setClient( Oscar::Client* );

	void connectToServer( const QString& host, quint16 port, bool encrypted, const QString &name );
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
	QList<int> supportedFamilies() const;

	/**
	 * Add the SNAC families in \p familyList to the list of supported families for
	 * this connection
	 * \param familyList the list of families to add
	 */
	void addToSupportedFamilies( const QList<int>& familyList );

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
	void addToRateClasses( const QList<RateClass*> rateClassList );

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
	 * \param s the SNAC the error occurred from
	 * \param errCode the error code
	 */
	void taskError( const Oscar::SNAC& s, int errCode );

	/**
	 * Indicate to the connection that there has been a fatal error in a task.
	 * This error will require a disconnection from the OSCAR service and if
	 * necessary, the user should be prompted to reconnect manually or an
	 * automatic reconnection should be attempted.
	 * \param s the SNAC the error occurred from
	 * \param errCode the error code
	 */
	void fatalTaskError( const Oscar::SNAC& s, int errCode );

    /**
     * Get the chat room name for this connection.
     * @return the name of the room or QString() if not connected to a room
     */

	/** Get the user settings object */
	Oscar::Settings* settings() const;

	/** Get the current FLAP sequence for this connection */
	Oscar::WORD flapSequence();

	/** Get the current SNAC sequence for this connection
	 * @note for historical reasons the internal counter is WORD only as we used it for
	 * long time as WORD on many places. If should be save to change it to DWORD for our
	 * Oscar implementation but Oscar protocol could brake especially on higher values.
	 */
	Oscar::DWORD snacSequence();

	/** Get the cookie for this connection */
	QByteArray cookie() const;

	QString userId() const;
	QString password() const;
	bool isIcq() const;
	ContactManager* ssiManager() const;
	const Oscar::ClientVersion* version() const;
	Oscar::Guid versionCap() const;
	RateClassManager* rateManager() const;
	bool isLoggedIn() const;

	QHostAddress localAddress() const;

	/** Map snac sequence to message info */
	void addMessageInfo( Oscar::DWORD snacSequence, const Oscar::MessageInfo& messageInfo );

	/** Return message info for snac sequence and remove it from map */
	Oscar::MessageInfo takeMessageInfo( Oscar::DWORD snacSequence );

	/** Return message info list */
	QList<Oscar::MessageInfo> messageInfoList() const;

	/** Convenience function to get the root task for use in Tasks */
	Task* rootTask() const;

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

	/** Generates initial flap sequence number as ICQ 6 */
	Oscar::WORD generateInitialFlapSequence() const;

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
	static QList<Oscar::WORD> m_startFlapSequenceList;
	ConnectionPrivate* d;
	bool m_loggedIn;
};

#endif

//kate: tab-width 4; indent-mode csands; auto-insert-doxygen on;
