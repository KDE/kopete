// from xmpp.h

#ifndef GW_CLIENTSTREAM_H
#define GW_CLIENTSTREAM_H

#include <qca.h>

#include "gwfield.h"
#include "stream.h"

// forward defines
class ByteStream;
class Connector;
class Request;
class TLSHandler;

typedef struct NovellDN
{
	QString dn;
	QString server;
};

class ClientStream : public Stream
{
	Q_OBJECT
	class Buffer : public QByteArray
	{
	public:
		Buffer( unsigned size = 0 );
		~Buffer();
		void add( char *str, unsigned size );
		QByteArray take( unsigned size );
	};

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
/*#		WarnOldVersion,             // server uses older XMPP/Jabber "0.9" protocol  // can be customised for novell versions*/
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

	ClientStream(Connector *conn, TLSHandler *tlsHandler=0, QObject *parent=0);
	ClientStream(const QString &host, const QString &defRealm, ByteStream *bs, QCA::TLS *tls=0, QObject *parent=0); // server
	~ClientStream();

/*#	Jid jid() const;	// novell DN*/
	void connectToServer(const NovellDN &id, bool auth=true);
	void accept(); // server
	bool isActive() const;
	bool isAuthenticated() const;

	// login params
	void setUsername(const QString &s);
	void setPassword(const QString &s);
	void setRealm(const QString &s);
	void continueAfterParams();

	// SASL information
// 	QString saslMechanism() const;
// 	int saslSSF() const;

	// bindingvoid ClientStream::setUsername(const QString &s)
// {
// 	if(d->sasl)
// 		d->sasl->setUsername(s);
// }


	// security options (old protocol only uses the first !)
	void setAllowPlain(bool);
	void setRequireMutualAuth(bool);
// 	void setSSFRange(int low, int high);
// 	void setOldOnly(bool);
// 	void setSASLMechanism(const QString &s);
	void setLocalAddr(const QHostAddress &addr, Q_UINT16 port);

	// reimplemented
// #	QDomDocument & doc() const; // jabber specific
// #	QString baseNS() const;		// "
// #	bool old() const;			// no idea?

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
	void readyRead(); //signals that there is a transfer ready to be read
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

// 	void sasl_clientFirstStep(const QString &mech, const QByteArray *clientInit);
// 	void sasl_nextStep(const QByteArray &stepData);
// 	void sasl_needParams(bool user, bool authzid, bool pass, bool realm);
// 	void sasl_authCheck(const QString &user, const QString &authzid);
// 	void sasl_authenticated();
// 	void sasl_error(int);

	void doNoop();
	void doReadyRead();

private:
	class Private;
	Private *d;

	void reset(bool all=false);
	void processNext();
/*	int convertedSASLCond() const;*/
	bool handleNeed();
	void handleError();
	void srvProcessNext();
	
	/** 
	 * convert internal method representation to wire
	 */
	static char* encode_method(Q_UINT8 method);
};

#endif
