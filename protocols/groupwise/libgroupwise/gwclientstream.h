/*
    gwclientstream.h - Kopete Groupwise Protocol
  
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

#ifndef GW_CLIENTSTREAM_H
#define GW_CLIENTSTREAM_H

#include <QHostAddress>

#include <QtCrypto>

#include "gwfield.h"
#include "libgroupwise_export.h"
#include "stream.h"

// forward defines
class Connector;
class Request;
class TLSHandler;

struct NovellDN
{
	QString dn;
	QString server;
};

class LIBGROUPWISE_EXPORT ClientStream : public Stream
{
	Q_OBJECT
public:
	enum Error {
		ErrConnection = ErrCustom,  // Connection error, ask Connector-subclass what's up
		ErrNeg,                     // Negotiation error, see condition
		ErrTLS,                     // TLS error, see condition
		ErrAuth,                    // Auth error, see condition
		ErrSecurityLayer,           // broken SASL security layer
		ErrBind                     // Resource binding error
	};
	enum Warning {
/*#		WarnOldVersion,             // server uses older XMPP/Jabber "0.9" protocol  // can be customized for novell versions*/
		WarnNoTLS                   // there is no chance for TLS at this point
	};
	enum NegCond {
		HostGone,                   // host no longer hosted
		HostUnknown,                // unknown host
		RemoteConnectionFailed,     // unable to connect to a required remote resource
		SeeOtherHost,               // a 'redirect', see errorText() for other host
		UnsupportedVersion          // unsupported XMPP version
	};
	enum TLSCond {
		TLSStart,                   // server rejected STARTTLS
		TLSFail                     // TLS failed, ask TLSHandler-subclass what's up
	};
	enum SecurityLayer {
		LayerTLS,
		LayerSASL
	};
	enum AuthCond {
		GenericAuthError,           // all-purpose "can't login" error
		NoMech,                     // No appropriate auth mech available
		BadProto,                   // Bad SASL auth protocol
		BadServ,                    // Server failed mutual auth
		EncryptionRequired,         // can't use mech without TLS
/*#		InvalidAuthzid,             // bad input JID  // need to change this to novell DN*/
		InvalidMech,                // bad mechanism
		InvalidRealm,               // bad realm
		MechTooWeak,                // can't use mech with this authzid
		NotAuthorized,              // bad user, bad password, bad creditials
		TemporaryAuthFailure        // please try again later!
	};
	enum BindCond {
		BindNotAllowed,             // not allowed to bind a resource
		BindConflict                // resource in-use
	};

	explicit ClientStream(Connector *conn, TLSHandler *tlsHandler=0, QObject *parent=0);
	~ClientStream();

	void connectToServer(const NovellDN &id, bool auth=true);
	void accept(); // server
	bool isActive() const;
	bool isAuthenticated() const;

	// login params
	void setUsername(const QString &s);
	void setPassword(const QString &s);
	void setRealm(const QString &s);
	void continueAfterParams();

	// security options (old protocol only uses the first !)
	void setAllowPlain(bool);
	void setRequireMutualAuth(bool);
	void setLocalAddr(const QHostAddress &addr, quint16 port);

	void close();
	
	/**
	 * Are there any messages waiting to be read
	 */
	bool transfersAvailable() const;
	/**
	 * Read a message received from the server
	 */
	Transfer * read();

	/**
	 * Send a message to the server
	 */
	void write( Request * request );

	int errorCondition() const;
	QString errorText() const;
// #	QDomElement errorAppSpec() const; // redondo

	// extrahttp://bugs.kde.org/show_bug.cgi?id=85158
/*#	void writeDirect(const QString &s); // must be for debug testing*/
	void setNoopTime(int mills);

signals:
	void connected();
	void securityLayerActivated(int);
	//void needAuthParams(bool user, bool pass, bool realm);
	void authenticated(); // this signal is ordinarily emitted in processNext
	void warning(int);
// #	void incomingXml(const QString &s); // signals emitted in processNext but don't seem to go anywhere...
// #	void outgoingXml(const QString &s); //
//	void readyRead(); //signals that there is a transfer ready to be read - defined in stream
public slots:
	void continueAfterWarning();

private slots:
	void cr_connected();
	void cr_error();
	/**
	 * collects wire ready outgoing data from the core protocol and sends
	 */ 
	void cp_outgoingData( const QByteArray& );
	/**
	 * collects parsed incoming data as a transfer from the core protocol and queues
	 */
	void cp_incomingData();

	void bs_connectionClosed();
	void bs_delayedCloseFinished();
	void bs_error(int); // server only

	void ss_readyRead();
	void ss_bytesWritten(int);
	void ss_tlsHandshaken();
	void ss_tlsClosed();
	void ss_error(int);

	void doNoop();
	void doReadyRead();

private:
	class Private;
	Private * const d;

	void reset(bool all=false);
	void processNext();
	bool handleNeed();
	void handleError();
	void srvProcessNext();
	
	/** 
	 * convert internal method representation to wire
	 */
	static char* encode_method(quint8 method);
};

#endif
