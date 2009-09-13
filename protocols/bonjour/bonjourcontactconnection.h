/*
    bonjourcontactconnection.h - Kopete Bonjour Protocol

    Copyright (c) 2007      by Tejas Dinkar	<tejas@gja.in>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef BONJOURCONTACTCONNECTION_H
#define BONJOURCONTACTCONNECTION_H

#include <QTcpSocket>
#include <QString>
#include <QHostAddress>
#include <QByteArray>
#include <QXmlStreamReader>

#include <kopetemessage.h>

/**
 * @brief This Represents a connection to a contact
 *
 * The BonjourContactConnection contains a @ref socket which has a connection to some contact.
 * Data from the @ref socket is parsed by the QXmlStreamReader @ref parser.
 * We Get @ref BonjourXmlToken tokens one at a time using the @ref getNextToken() functions
 * All Jabber specific code must be here ONLY
 *
 * There is a protocol document that nobody follows. This can be found here:
 * http://www.xmpp.org/extensions/attic/jep-0174-0.5.html
 * The standard is totally sucky, and I'm not surprised that there isn't a single client who
 * follows it faithfully (that I know of). The standard `we` follow was derived from wireshark
 * logs with a machine running Pidgen.
 *
 * Pass Two Strings for an outgoing connection (second constructor)
 *
 * @author Tejas Dinkar <tejas\@gja.in>
 */
class BonjourContactConnection : public QObject {

	Q_OBJECT

	/**
	 * @brief The State of the Connection
	 * 
	 * @ref connectionState holds this state
	 */
	enum BonjourConnectionState {
		BonjourConnectionNewOutgoing,		// New Outgoing Stream
		BonjourConnectionNewIncoming,		// New Incoming Stream

		BonjourConnectionOutgoingStream,	// Expect a <stream>
		BonjourConnectionToWho,			// We are Unsure who we connect to

		BonjourConnectionConnected = 50,	// Connected
		BonjourConnectionDisconnected,		// Disconnected

		BonjourConnectionError = 99		// Reserved for Future Use
	} connectionState;

	// The Actual Connection
	QTcpSocket *socket;

	// The XML Parser
	QXmlStreamReader parser;

	// The local and remote names 
	QString local;
	
	// The local and remote names 
	QString remote;

	// Set the Socket
	void setSocket(QTcpSocket *socket);

	// Determine if there is more data available
	bool moreTokensAvailable();

	/**
	 * @brief Description of A Token
	 *
	 * Internally used structures to describe a token as obtained by the parser
	 * Not all of these names represent valid token names. Some are just provided as a convenient
	 * way to call @ref getNextToken()
	*/
	enum BonjourXmlTokenName {
		BonjourXmlTokenOther,
		BonjourXmlTokenNone,
		BonjourXmlTokenStream,
		BonjourXmlTokenMessage,
		BonjourXmlTokenBody,
		BonjourXmlTokenHtml,
		BonjourXmlTokenX,
		BonjourXmlTokenIq,
		BonjourXmlTokenQuery,

		BonjourXmlStartElement = 50,		// Simply A Convenient Parameter to getNextToken
		BonjourXmlEndElement,			// No Token is ever of this type
		BonjourXmlStartOrEndElement,		// Instead see the value of 'type' (QXmlStreamReader::TokenType)

		BonjourXmlTokenError = 99
	};
	
	/*
	 * @brief table to translate between xml tokens and corresponding @BonjourXmlTokenName
	 *
	 * This is a Hash Object to Translate a qualified name into a BonjourXmlTokenName quickly
	 * Only one instance is needed. We derive from QHash and fill it in the constructor
	 */
	static class TokenTable : public QHash <QString, BonjourXmlTokenName>{
	    public:
		TokenTable();
	} tokenTable;

	/**
	 * @brief This Describes a Token
	 * 
	 * This Describes a Token. It contains a number of useful attributes like type, name, etc..
	 */
	struct BonjourXmlToken {
		QXmlStreamReader::TokenType type;
		BonjourXmlTokenName name;
		QStringRef qualifiedName;
		QXmlStreamAttributes attributes;
		QStringRef text;
	};

	/**
	 * @brief Get the Next Token Read in Stream
	 *
	 * This Next Function Gets the Next Token in the stream
	 * This may be of any time. We recommend you pass it a parameter of what type(s)
	 * of tokens that you expect
	 *
	 * @return the token. If error, token.name will be BonjourXmlTokenError
	 */
	const BonjourXmlToken getNextToken();

	/**
	 * @brief Get Next Token Of a Certain Type
	 *
	 * This Function returns the next token whose name matches. All tokens before this
	 * is found are simply thrown away.
	 * You May Also Search for Next Start Element or End Element (see @ref BonjourXmlTokenName)
	 *
	 * @param name The Name of the Next Token We want
	 * @return the token. If error, token.name will be BonjourXmlTokenError
	 */
	const BonjourXmlToken getNextToken(BonjourXmlTokenName name);

	/**
	 * @brief This is called if we receive an iq packet
	 *
	 * iq is only sent by clients like miranda... Who don't care about the reply anyway.
	 * Later, we may implement support for iq packets. For now we ignore them.
	 *
	 * @todo FIXME: Currently we ignore everything between <iq> and </iq>
	 * @param token The Token Containing the iq. Will be erased
	 */
	void ignoreAllIq(BonjourXmlToken &token);


    public:

	/**
	 * @brief Constructor For Incoming Connections
	 * 
	 * @param aSocket The Socket To Wrap Up
	 */
	explicit BonjourContactConnection(QTcpSocket *aSocket, QObject *parent = NULL);

	/**
	 * @brief Constructor For Outgoing Connection
	 * 
	 * This is For OutGoing Connections
	 * We Create a Socket Here, so this may take upto 3 seconds
	 *
	 * @param address The Address of the Remote Contact
	 * @param port The Port the contact is listening on
	 * @param remote The remoteuser@hostname
	 * @param local myself@hostname
	 */
	BonjourContactConnection(const QHostAddress &address, short int port, const QString &alocal, const QString &aremote, QObject *parent = NULL);

	// Destructor
	~BonjourContactConnection();

	/**
	 * @brief Write <stream:stream> in socket
	 *
	 * If the values of remote and local are set, we encode it in the stream as well
	 */
	void sayStream();

	/**
	 * @brief Examine the Next Token, and perform action
	 *
	 * This Examines the Data From The Socket and emits a messageReceived (maybe)
	 * The Parameter is the most recent token.
	 *
	 * @param token The Most Recent Token. The value of this token is probably destroyed
	 */
	void readData(BonjourXmlToken &token);

	/**
	 * @brief Called when we expect a stream token
	 * 
	 * This is Called When a <stream> is expected
	 * The token should contain <stream>. If it doesn't, then we keep getting new tokens until
	 * we either get a <stream> or we get an EOF
	 *
	 * This emits either a @ref discoveredUserName() or @ref usernameNotInStream()
	 *
	 * @param token The token which probably contains the <stream>
	 */
	void getStreamTag(BonjourXmlToken &token);

	/**
	 * @brief Called When we are connected, but don't know to who
	 *
	 * This is called when we are waiting for the remote and local values (incoming connections only).
	 * Honestly, this function is never used, as either signal emitted by @ref getSteam() may be used
	 * to uniquely identify a contact
	 *
	 * @todo FIXME: Currently unimplemented as (at least gaim) says who it is in the <stream>
	 */
	void getWho(BonjourXmlToken &token);

	/**
	 * @brief Gets the Address of the Current Connection
	 *
	 * @return The HostAddress of the Remote Contact
	 */
	QHostAddress getHostAddress();

	/**
	 * @brief Create a new Message
	 *
	 * This Creates a Message
	 *
	 * @param direction The Direction of the Message
	 * @return The Message
	 */
	Kopete::Message newMessage(Kopete::Message::MessageDirection direction);

	/**
	 * @brief Get a message from stream of tokens
	 *
	 * This reads all tokens upto (and including) a </message>, and attempts to assemble
	 * a message out of the tokens. This emits a @ref messageReceived() signal if a message
	 * can be read.
	 *
	 * @todo FIXME: HTML messages are ignored
	 *
	 * @param token A Token which contains a <message>. It's value will be destroyed
	 */
	void readMessage(BonjourXmlToken &token);

	/**
	 * @brief Check if a Connection is ready for general data transfer
	 * @return @c true if it is connected, @c false otherwise
	 */
	inline bool isConnected()
	{
		return connectionState == BonjourConnectionConnected;
	}

	/**
	 * @brief Send a </stream>
	 */
	void sayGoodBye();

	/**
	 * @brief Set @ref remote and @ref local strings externally
	 *
	 * This function is usually called if the contact is identified via an
	 * IP lookup, and not via the stream element.
	 *
	 * @param remote The Remote User
	 * @param local The Local User
	 */
	void setRemoteAndLocal(const QString &remote, const QString &local);

    signals:

	/**
	 * @brief This Signal is Emitted when there is new data
	 *
	 * @todo FIXME: This is unused. Remove it
	 */
	void newData(BonjourContactConnection *);

	/**
	 * @brief Signal Emitted when we discover who the remote user is
	 *
	 * This signal is emitted when we discover who we are connected to (unverified)
	 * Used in Incoming Connections Only
	 *
	 * The Username is sent back
	 */
	void discoveredUserName(BonjourContactConnection *, const QString &);

	/**
	 * @brief Send if the stream tag didn't contain a from and to
	 *
	 * This signal is sent if we can't figure out who it is
	 */
	void usernameNotInStream(BonjourContactConnection *);

	/**
	 * @brief This signal is sent if the @ref socket is disconnected
	 *
	 * This Signal is basically forwarding disconnect signal from socket
	 */
	void disconnected(BonjourContactConnection *);

	/**
	 * @brief Signal Emitted when a new message has been received (already formatted)
	 */
	void messageReceived(Kopete::Message);

	/**
	 * @brief Signal Emitted if we could not connect to an external socket
	 */
	void errorCouldNotConnect();

    public slots:
	
	/**
	 * @brief This slot is called by the socket, to signify new data
	 */
	void dataInSocket();

	/**
	 * @brief This is Called when the socket disconnects
	 *
	 * This basically emits @ref disconnected()
	 */
	void socketDisconnected();

	/**
	 * @brief Send a message
	 */
	void sendMessage(const Kopete::Message &message);
};

#endif
