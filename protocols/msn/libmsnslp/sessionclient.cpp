/*
    sessionclient.cpp - Peer to Peer Session Client

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "sessionclient.h"
#include "application.h"
#include "dialog.h"
#include "filetransfersession.h"
#include "session.h"
#include "sessionnotifier.h"
#include "transport.h"
#include "udisession.h"

#include <qdom.h>
#include <qfile.h>
#include <qhostaddress.h>
#include <qregexp.h>
#include <qtextstream.h>

#include <kmdcodec.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kmdcodec.h>

#include <arpa/inet.h>
#include <stdlib.h>


namespace PeerToPeer
{

class SessionClient::SessionClientPrivate
{
	public:
		QMap<QUuid, Application*> applications;
		QMap<QUuid, Dialog*> dialogs;
		QString externalIpAddress;
		QString internalIpAddress;
		QString myUri;
		QString peerUri;
		QMap<Q_INT32, Session*> sessions;
		QMap<QUuid, Transaction*> transactions;
		Transport* transport;
		QString version;
};

SessionClient::SessionClient(const QMap<QString, QVariant> & properties, Transport* transport, QObject *parent) : QObject(parent), d(new SessionClientPrivate())
{
	// Configure the client using the supplied properties
	d->externalIpAddress = properties["externalIpAddress"].toString();
	d->internalIpAddress = properties["internalIpAddress"].toString();
	d->myUri = properties["myUri"].toString();
	d->peerUri = properties["peerUri"].toString();
	// Set the supported protocol version.
	d->version = QString::fromLatin1("1.0");
	// Set the transport for the client.
	d->transport = transport;
	// Initialize the client.
	initialize();
}

SessionClient::~SessionClient()
{
	delete d;
	d = 0l;
}

void SessionClient::initialize()
{
	SessionNotifier *handler  = new SessionNotifier(this);
	// Connect the signal/slot
	QObject::connect(handler, SIGNAL(messageReceived(const QByteArray&, const Q_INT32, const Q_INT32)), this,
	SLOT(onReceived(const QByteArray&, const Q_INT32, const Q_INT32)));
	QObject::connect(handler, SIGNAL(messageAcknowledged(const Q_INT32)), this,
	SLOT(onSend(const Q_INT32)));
	QObject::connect(handler, SIGNAL(transactionTimedout(const Q_INT32, const Q_INT32)), this,
	SLOT(onTransactionTimedout(const Q_INT32, const Q_INT32)));
	// Register the handler for the session client , session 0
	d->transport->registerReceiver(0, handler);

	handler = new SessionNotifier(this);
	// Connect the signal/slot
	QObject::connect(handler, SIGNAL(messageReceived(const QByteArray&, const Q_INT32, const Q_INT32)), this,
	SLOT(onImageReceived(const QByteArray&, const Q_INT32, const Q_INT32)));
	// Register the handler for the session client , session 64
	d->transport->registerReceiver(64, handler);

	handler = 0l;
}

const Q_INT32 SessionClient::activeDialogs() const
{
	return d->dialogs.size();
}

void SessionClient::createSession(const QUuid& uuid, const Q_UINT32 sessionId, const Q_UINT32 appId, const QString& context)
{
	// Create a new pending dialog.
	Dialog *dialog = new Dialog(QUuid::createUuid(), this);
	dialog->session = sessionId;

	// Build the dialog creating INVITE request.
	SlpRequest request = buildRequest("INVITE", "application/x-msnmsgr-sessionreqbody", dialog);
	// Build the session description body.
	const QString sessionDescriptionBody = buildSessionDescriptionBody(uuid, sessionId, appId, context);
	// Set the request body;
	request.setBody(sessionDescriptionBody);
	//Set the request context information.
	request.setIdentifier(dialog->transactionId());
	request.setRelatesTo(0);

	// Add the pending dialog to the call map.
	addDialogToCallMap(dialog);

	// Create a new transaction for the request.
	Transaction *transaction = new Transaction(request, true);
	dialog->setInitialTransaction(transaction);

	// Begin the transaction.
	beginTransaction(transaction);
}

void SessionClient::requestAvatar(const QString& object)
{
	UdiSession *session = new UdiSession((10 * (rand() % 0xC25C + 10)), Session::Incoming, this);
	QString user = d->peerUri;
	session->setDataStore(new QFile(::locateLocal("tmp", QString("msnpicture-%1.png").arg(user.replace('.', '-')))));
	// Connect the signal/slot
	QObject::connect(session, SIGNAL(transferComplete(const QString&)), this,
	SLOT(onUdiTransferComplete(const QString&)));

	// Add the session to the list of session.
	d->sessions.insert(session->id(), session);

	// Create the context field of the session description.
	const QString context = QString::fromUtf8(KCodecs::base64Encode(object.utf8()));
	// Create the uuid that indicates the type of session to create.
	const QUuid uuid = QUuid("A4268EEC-FEC5-49E5-95C3-F126696BDBF6");

	// Create the session to request the avatar object.
	createSession(uuid, session->id(), 12, context);
}

const QString SessionClient::buildSessionDescriptionBody(const QUuid& uuid, const Q_UINT32 sessionId, const Q_UINT32 appId, const QString& context)
{
	const QString CrLf = QString::fromLatin1("\r\n");

	QString sessionDescriptionBody;
	QTextStream(sessionDescriptionBody, IO_WriteOnly)
	<< QString::fromLatin1("EUF-GUID: ") << uuid.toString().upper() << CrLf
	<< QString::fromLatin1("SessionID: ") << sessionId << CrLf
	<< QString::fromLatin1("AppID: ") << appId << CrLf
	<< QString::fromLatin1("Context: ") << context << CrLf
	<< CrLf;

	return sessionDescriptionBody;
}

//BEGIN Helper Functions

QUuid SessionClient::getTransactionBranchFrom(const SlpMessage& message)
{
	QUuid branch;
	QRegExp regex("branch=\\{([0-9A-Fa-f\\-]*)\\}");
	QString viaHeaderValue = message.headers()["Via"].toString();
	if (regex.search(viaHeaderValue, false) != -1)
	{
		branch = QUuid(regex.cap(1));
	}

	return branch;
}

void SessionClient::parseHeaders(const QString& input, QMap<QString, QVariant> & headers)
{
	int i = 0;
	QRegExp regex("([^\r\n:]*):\\s([^\r\n]*)");
	while((i = regex.search(input, i)) != -1)
	{
		headers.insert(regex.cap(1), QVariant(regex.cap(2)));
		i += regex.matchedLength();
	}
}

//END

void SessionClient::onReceived(const QByteArray& content, const Q_INT32 identifier, const Q_INT32 relatesTo)
{
	const QString message(content);
	kdDebug() << "============= RECEIVED message=" << identifier << ", " << message << endl;

	// Try to parse the session layer protocol (slp)
	// message from the raw data received.
	QString startLine = message.section("\r\n", 0, 0);
	if (startLine.startsWith("INVITE") || startLine.startsWith("BYE"))
	{
		// If the start line contains one of the supported request methods,
		// try to parse the request from the raw data.
		QRegExp regex("^(\\w+) MSNMSGR:([\\w@.]*) MSNSLP/(\\d\\.\\d)");
		if (regex.search(startLine) != -1)
		{
			// Get the request method.
			const QString method = regex.cap(1);
			// Get the request uri.
			const QString requestUri = regex.cap(2);
			// Get the request message version.
			const QString version = regex.cap(3);
			// Create the request from the supplied method and request uri.
			SlpRequest request(method, requestUri, version);

			QMap<QString, QVariant> headers;
			// Parse the request headers from the raw data.
			parseHeaders(message.section("\r\n\r\n", 0, 0), headers);
			// Copy the parsed headers to the request's header collection.
			request.copyHeadersFrom(headers);

			if (request.contentLength() > 0)
			{
				// Set the request body content.
				const QString requestBody = message.section("\r\n\r\n", 1, 1, QString::SectionIncludeTrailingSep);
				request.setBody(requestBody);
			}

			// Set the message context information.
			request.setIdentifier(identifier);
			request.setRelatesTo(relatesTo);

			// Handle the received slp request.
			onRequestMessage(request);
		}
	}
	else
	if (startLine.startsWith("MSNSLP"))
	{
		// If the start line starts with the protocol string,
		// try to parse the response from the raw data.
		QRegExp regex("^MSNSLP/(\\d\\.\\d) ([0-9]{3}) ([A-Za-z ]*)");
		if (regex.search(startLine) != -1)
		{
			// Get the request message version.
			const QString version = regex.cap(1);
			// Get the response status code.
			const Q_UINT32 statusCode = regex.cap(2).toUInt();
			// Get the response status deescription
			const QString statusDescription = regex.cap(3);
			// Create the response from the supplied status code and description.
			SlpResponse response(statusCode, statusDescription, version);

			QMap<QString, QVariant> headers;
			// Parse the response headers from the raw data.
			parseHeaders(message.section("\r\n\r\n", 0, 0), headers);
			// Copy the parsed headers to the response's header collection.
			response.copyHeadersFrom(headers);

			if (response.contentLength() > 0)
			{
				// Set the response body content.
				const QString responseBody = message.section("\r\n\r\n", 1, 1, QString::SectionIncludeTrailingSep);
				response.setBody(responseBody);
			}

			// Set the message context information.
			response.setIdentifier(identifier);
			response.setRelatesTo(relatesTo);

			// Handle the received slp response.
			onResponseMessage(response);
		}
	}
	else
	{
		kdDebug() << k_funcinfo << "Unrecognized message=" << identifier
		<< " -- ignoring" << endl;
	}
}

void SessionClient::onSend(const Q_INT32 identifier)
{
	// ACK to sent request
	QMap<QUuid, Transaction*>::Iterator it = d->transactions.begin();
	while (it != d->transactions.end())
	{
		if (it.data()->request().identifier() == identifier)
		{
			Transaction *transaction = it.data();
			// Mark the transaction as confirmed by the peer endpoint.
			transaction->confirm();
			kdDebug() << "Dialog COMFIRMED transaction, " << transaction->identifier().toString().upper() << endl;

			if (transaction->request().method() == QString::fromLatin1("BYE"))
			{
				// Try to retrieve a terminating dialog associated with the BYE transaction.
				Dialog *dialog = getDialogByTransactionId(transaction->request().identifier());
				if (dialog != 0l && dialog->state() == Dialog::Terminating)
				{
					// If found, terminate the dialog.
					dialog->terminate();
				}
				// Finish the transaction.
				endTransaction(transaction);
			}
			return;
		}
		it++;
	}

	// ACK to sent response
	Dialog *dialog = getDialogByTransactionId(identifier);

	if (dialog != 0l && dialog->state() == Dialog::Pending)
	{
		// This ACK response, established the pending dialog.
		// so, set the dialog state to established.
		dialog->establish();
	}
}

//BEGIN Dialog Functions

void SessionClient::addDialogToCallMap(Dialog *dialog)
{
	if (dialog != 0l && !d->dialogs.contains(dialog->identifier()))
	{
		kdDebug() << k_funcinfo << "Adding dialog=" << dialog->identifier().toString().upper() << " to call map" << endl;
		// Connect the signal/slot.
		QObject::connect(dialog, SIGNAL(established()), this,
		SLOT(onDialogEstablish()));
		QObject::connect(dialog, SIGNAL(terminated()), this,
		SLOT(onDialogTerminate()));

		// Add the new dialog to the call map.
		d->dialogs.insert(dialog->identifier(), dialog);
	}
}

Dialog* SessionClient::getDialogByCallId(const QUuid& identifier)
{
	Dialog *dialog = 0l;
	if (d->dialogs.contains(identifier)) dialog = d->dialogs[identifier];
	return dialog;
}

Dialog* SessionClient::getDialogBySessionId(const Q_UINT32 sessionId)
{
	Dialog *dialog = 0l;
	QMap<QUuid, Dialog*>::Iterator it = d->dialogs.begin();
	while (it != d->dialogs.end())
	{
		if (it.data()->session == sessionId)
		{
			dialog = it.data();
			break;
		}
		it++;
	}

	return dialog;
}

Dialog* SessionClient::getDialogByTransactionId(const Q_UINT32 transactionId)
{
	Dialog *dialog = 0l;
	QMap<QUuid, Dialog*>::Iterator it = d->dialogs.begin();
	while (it != d->dialogs.end())
	{
		if (it.data()->transactionId() == transactionId)
		{
			dialog = it.data();
			break;
		}
		it++;
	}

	return dialog;
}

void SessionClient::onDialogEstablish()
{
	Dialog *dialog = static_cast<Dialog*>(const_cast<QObject*>(sender()));
	if (dialog != 0l)
	{
		kdDebug() << "Dialog ESTABLISHED, " << dialog->identifier().toString().upper() << endl;

		// TODO Update the session state information.

		// TODO Start the session.
	}
}

void SessionClient::onDialogTerminate()
{
	Dialog *dialog = static_cast<Dialog*>(const_cast<QObject*>(sender()));
	if (dialog != 0l)
	{
		kdDebug() << "Dialog TERMINATED, " << dialog->identifier().toString().upper() << endl;

		// Disconnect from the signal/slot
		QObject::disconnect(dialog, 0, this, 0);

		// TODO Stop the session if active.

		// Remove the dialog from the call map.
		removeDialogFromCallMap(dialog->identifier());
		// Delete the terminated dialog as its lifetime has expired.
		dialog->deleteLater();
		dialog = 0l;
	}
}

void SessionClient::removeDialogFromCallMap(const QUuid& identifier)
{
	if (d->dialogs.contains(identifier))
	{
		kdDebug() << k_funcinfo << "Removing dialog=" << identifier.toString().upper() << " from call map" << endl;
		// Remove the dialog from the call map.
		d->dialogs.remove(identifier);
	}
}

//END

//BEGIN Transaction Functions

void SessionClient::beginTransaction(Transaction *transaction)
{
	kdDebug() << "Dialog BEGIN transaction, " << transaction->identifier().toString().upper() << endl;

	d->transactions.insert(transaction->identifier(), transaction);
	QObject::connect(transaction, SIGNAL(timeout()), this, SLOT(onTransactionTimeout()));
	transaction->begin();

	// Send the transaction request.
	send(transaction->request(), 0);
}

void SessionClient::endTransaction(Transaction *transaction)
{
	kdDebug() << "Dialog END transaction, " << transaction->identifier().toString().upper() << endl;

	// Remove the transaction from the list of active transactions.
	d->transactions.remove(transaction->identifier());
	QObject::disconnect(transaction, SIGNAL(timeout()), this, SLOT(onTransactionTimeout()));
	transaction->end();
}

// TODO
void SessionClient::onTransactionTimedout(const Q_INT32 identifier, const Q_INT32 relatesTo)
{
	kdDebug() << k_funcinfo << "Got a transaction timeout -- " << identifier << endl;
}

void SessionClient::onTransactionTimeout()
{
	Transaction *transaction = dynamic_cast<Transaction*>(const_cast<QObject*>(sender()));
	if (transaction != 0l)
	{
		kdDebug() << k_funcinfo << "Dialog transaction TIMED OUT " << transaction->identifier().toString().upper() << endl;
		Dialog *dialog = getDialogByTransactionId(transaction->request().identifier());
		if (dialog != 0l && (dialog->state() == Dialog::Pending || dialog->state() == Dialog::Terminating))
		{
			// TODO Send a transaction timed out response to the remote endpoint

			// Terminate the dialog.
			dialog->terminate();
		}

		// End the transaction.
		endTransaction(transaction);
	}
}

//END

SlpRequest SessionClient::buildRequest(const QString& method, const QString& contentType, Dialog *dialog)
{
	// Generate the request
	SlpRequest request(method, d->peerUri);
	QMap<QString, QVariant> & headers = request.headers();
	// Set the request headers
	headers["To"] = QString("<msnmsgr:%1>").arg(d->peerUri);
	headers["From"] = QString("<msnmsgr:%1>").arg(d->myUri);
	headers["Via"] = QString("MSNSLP/1.0/TLP ;branch=%1").arg(QUuid::createUuid().toString().upper());
	headers["CSeq"] = 0;
	headers["Call-ID"] = dialog->identifier().toString().upper();
	headers["Max-Forwards"] = 0;
	headers["Content-Type"] = contentType;

	return request;
}

SlpResponse SessionClient::buildResponse(const Q_INT32 statusCode, const QString& statusDescription, const QString& contentType, const SlpRequest& request)
{
	// Generate the response
	SlpResponse response(statusCode, statusDescription);
	QMap<QString, QVariant> & headers = response.headers();
	// Set the response headers
	headers["To"] = request.headers()["From"];
	headers["From"] = request.headers()["To"];
	headers["Via"] = request.headers()["Via"];
	headers["CSeq"] = 1;
	headers["Call-ID"] = request.headers()["Call-ID"];
	headers["Max-Forwards"] = 0;
	headers["Content-Type"] = contentType;

	return response;
}

void SessionClient::acceptSession(Dialog *dialog, const Q_INT32 sessionId)
{
	// Generate a 200 OK response
	const SlpRequest & request = dialog->initialTransaction()->request();
	SlpResponse response = buildResponse(200, "OK", "application/x-msnmsgr-sessionreqbody", request);
	// Set the response body.
	const QString responseBody = QString("SessionID: %1\r\n\r\n").arg(sessionId);
	response.setBody(responseBody);
	// Set the response context information.
	response.setIdentifier(dialog->transactionId(true));
	response.setRelatesTo(request.identifier());

	// Send the 200 OK status response message.
	send(response, 0);
}

void SessionClient::declineSession(Dialog *dialog, const int sessionId)
{
	// Generate a 603 DECLINE response
	const SlpRequest & request = dialog->initialTransaction()->request();
	SlpResponse response = buildResponse(603, "DECLINE", "application/x-msnmsgr-sessionreqbody", request);
	// Set the response body.
	const QString responseBody = QString("SessionID: %1\r\n\r\n").arg(sessionId);
	response.setBody(responseBody);
	// Set the response context information.
	response.setIdentifier(dialog->transactionId(true));
	response.setRelatesTo(request.identifier());

	// Send the 603 DECLINE status response message.
	send(response, 0);
}

void SessionClient::endSession(Dialog *dialog, const int sessionId)
{
	// Build the INVITE request.
	SlpRequest request = buildRequest("BYE", "application/x-msnmsgr-sessionclosebody", dialog);
	// Set the request body.
	const QString requestBody = QString("SessionID: %1\r\n\r\n").arg(sessionId);
	request.setBody(requestBody);
	//Set the request context information.
	request.setIdentifier(dialog->transactionId(true));
	request.setRelatesTo(0);

	// Create a new transaction for the request.
	Transaction *transaction = new Transaction(request, true);
	dialog->transactions().append(transaction);
	// Begin the transaction.
	beginTransaction(transaction);
}

//BEGIN UA Server Functions

//////////////////////////////////////////////////////////////////////
// Handles session layer protocol requests received
// Slp only supports the INVITE and BYE methods.
//////////////////////////////////////////////////////////////////////
void SessionClient::onRequestMessage(const SlpRequest& request)
{
	if (request.to() != d->myUri)
	{
		kdDebug() << k_funcinfo << "Got request which is not for me -- responding with 404" << endl;

		// Send an acknowledge message to the peer.
		sendAcknowledge(0, request.relatesTo() + 1, request.identifier());

		// Generate a Not Found response
		SlpResponse response = buildResponse(404, "Not Found", "null", request);
		// Set the response context information.
		response.setIdentifier(request.relatesTo() + 2);
		response.setRelatesTo(request.identifier());

		// Send the 404 Not Found status response message.
		send(response, 0);
		return;
	}

	if (request.method() == QString::fromLatin1("INVITE"))
	{
		// Get the content type of the request.
		const QString contentType = request.contentType();
		// INVITE MSNMSGR:muser MSNSLP/1.0
		if (contentType == QString::fromLatin1("application/x-msnmsgr-sessionreqbody"))
		{
			// Handle the initial dialog creating invite request.
			onInitialInviteRequest(request);
		}
		else
		if (contentType == QString::fromLatin1("application/x-msnmsgr-transreqbody"))
		{
			// Handle the direct connection setup request.
			onDirectConnectionSetupRequest(request);
		}
		else
		if (contentType == QString::fromLatin1("application/x-msnmsgr-transrespbody"))
		{
			// Handle the direct connection offer request.
			onDirectConnectionOfferRequest(request);
		}
		else
		{
			kdDebug() << k_funcinfo << "Got unknown content-type on INVITE request -- responding with 500" << endl;

			// Send an acknowledge message to the peer.
			sendAcknowledge(0, request.relatesTo() + 1, request.identifier());

			// Generate a No Such Call response
			SlpResponse response = buildResponse(500, "Internal Error", "null", request);
			// Set the response context information.
			response.setIdentifier(request.relatesTo() + 2);
			response.setRelatesTo(request.identifier());

			// Send the 500 Internal Error status response message.
			send(response, 0);
		}
	}
	else
	if (request.method() == QString::fromLatin1("BYE"))
	{
		// BYE MSNMSGR:muser MSNSLP/1.0

		// Handle the bye request.
		onByeRequest(request);
	}
}

// TODO
bool SessionClient::parseSessionCloseBody(const QString& requestBody, QMap<QString, QVariant> & collection)
{
	QRegExp regex("Context: ([0-9a-zA-Z+/=]*)");
	// Check if the context field is valid.
	if (regex.search(requestBody) != -1)
	{
		// TODO If we cannot convert the context from
		// a bse64 string to bytes, return true.
		collection.insert("Context", regex.cap(1));
	}

	return false;
}

void SessionClient::onByeRequest(const SlpRequest& request)
{
	Dialog *dialog = getDialogByCallId(QUuid(request.headers()["Call-ID"].toString()));
	if (dialog == 0l)
	{
		kdDebug() << k_funcinfo << "Got unknown call id on BYE request -- responding with 481" << endl;

		// Generate a No Such Call response
		SlpResponse response = buildResponse(481, "No Such Call", "application/x-msnmsgr-session-failure-respbody", request);
		// Set the response body.
		const QString responseBody = QString::fromLatin1("SessionID: 0\r\n\r\n");
		response.setBody(responseBody);
		// Set the response context information.
		response.setIdentifier(request.relatesTo() + 1);
		response.setRelatesTo(request.identifier());

		// Send the 481 No Such Call status response message.
		send(response, 0);
	}
	else
	if (dialog != 0l && dialog->state() != Dialog::Terminated)
	{
		// Check whether the content type for the BYE request is correct.
		if (request.contentType() != QString::fromLatin1("application/x-msnmsgr-sessionclosebody"))
		{
			kdDebug() << k_funcinfo << "Got unknown content type on BYE request -- responding with 500" << endl;

			// Generate an Internal Error response
			SlpResponse response = buildResponse(500, "Internal Error", "null", request);
			// Set the response context information.
			response.setIdentifier(dialog->transactionId(true));
			response.setRelatesTo(request.identifier());

			// Send the 500 Internal Error status response message.
			send(response, 0);
		}
		else
		{
			// Otherwise, we have received a valid BYE request.
			kdDebug() << k_funcinfo << "Got BYE request for dialog=" << dialog->identifier().toString().upper() << endl;

			// Send an acknowledge to the remote endpoint.
			sendAcknowledge(0, dialog->transactionId(true), request.identifier());

			QMap<QString, QVariant> sessionCloseBody;
			if (parseSessionCloseBody(request.body(), sessionCloseBody))
			{
				return;
			}

			// Terminate the dialog.
			dialog->terminate();
		}
	}
}

// TODO
bool SessionClient::parseDirectConnectionRequestBody(const QString& requestBody, QMap<QString, QVariant> & collection)
{
	Q_UNUSED(requestBody);
	Q_UNUSED(collection);

	return false;
}

void SessionClient::onDirectConnectionSetupRequest(const SlpRequest& request)
{
	// Determine if an existing dialog exists.
	Dialog *dialog = getDialogByCallId(QUuid(request.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Established)
	{
		// Check if this transaction for a Direct Connection
		// setup overlaps with one that I recently sent.
		if (isMyDirectConnectionSetupRequestLoser(dialog, request) == false)
		{
			return;
		}

		kdDebug() << k_funcinfo << "Got Direct Connection setup INVITE request" << endl;

		// Get the request body.
		const QString & requestBody = request.body();

		// Parse the direct connection description from the request body
		QRegExp regex("Bridges: ([a-zA-Z0-9 ]*)\r\n");
		regex.search(requestBody);
		////////////////////////////////////////////
		// Supported bridge types: TRUDPv1 or TCPv1
		////////////////////////////////////////////
		QValueList<QString> supportedBridgeTypes = QStringList::split(" ", regex.cap(1));

		regex = QRegExp("NetID: (\\-?\\d+)\r\n");
		regex.search(requestBody);
		//////////////////////////////////////////////////////////////////////////////////
		// NetID is the big endian version of the sender's internet visible (external)
		// ip address.  For connection types Direct-Connect, Unknown-Connect and Firewall
		// however, NetID is always 0.
		//////////////////////////////////////////////////////////////////////////////////
		const Q_UINT32 netId = regex.cap(1).toInt();
		const QHostAddress peerIpAddress(ntohl(netId));

		regex = QRegExp("UPnPNat: ([a-z]*)\r\n");
		regex.search(requestBody);
		//////////////////////////////////////////////////////////////////
		// Indicates whether the peer is connected to the internet
		// through a router that supports the Universal Plug and Play
		// (UPnp) specification.
		//////////////////////////////////////////////////////////////////
		const bool uPnpSupported = (QString::fromLatin1("true") == regex.cap(1));

		regex = QRegExp("Conn-Type: ([a-zA-Z\\-]*)\r\n");
		regex.search(requestBody);

		///////////////////////////////////////////////////////////////////////////////////////////
		// Connection Types
		// Direct-Connect		The peer is directly connected to the internet
		// IP-Restrict-NAT		The peer is connected to the internet through an IP restricted NAT
		// Port-Restrict-NAT	The peer is connected to the internet through a port restricted NAT
		// Symmetric-NAT		The peer is connected to the internet through a symmetric NAT
		// Unknown-NAT			The peer is connected to the internet through an NAT is unknown.
		// Firewall				The peer is behind a firewall.
		// Unknown-Connect		The peer's connection scheme to the internet unknown.
		///////////////////////////////////////////////////////////////////////////////////////////
		const QString connectionType = regex.cap(1);

		regex = QRegExp("SessionID: (\\d+)");
		regex.search(requestBody);
		const QString identifier = regex.cap(1);

		regex = QRegExp("ICF: ([a-z]*)\r\n");
		regex.search(requestBody);

		/////////////////////////////////////////////////////
		// Indicates whether the peer is behind an internet
		// connection firewall.
		/////////////////////////////////////////////////////
		const bool behindFirewall = (QString::fromLatin1("true") == regex.cap(1));

		// TODO Handle Hashed-Nonce field
		QUuid nonce;

		// Send an acknowledge to the remote endpoint.
		sendAcknowledge(0, dialog->transactionId(true), request.identifier(), 0);

		// Indicate whether the peer endpoint is on the same network as us.
		const bool sameNetwork = (peerIpAddress.toString() == d->externalIpAddress);

		SlpResponse response;
		// Check the peer's direct connectivity support.
		bool allowDirect = supportsDirectConnectivity(connectionType, behindFirewall, uPnpSupported, sameNetwork);
		if (allowDirect && supportedBridgeTypes.contains("TCPv1") && false)
		{
			// If the peer's connectivity scheme is good enough to establish
			// a direct connection, accept the DCC setup request.
			response = buildResponse(200, "OK", "application/x-msnmsgr-transrespbody", request);
			// Build the direct connection response description.
			const QString dccResponseBody = buildDirectConnectionSetupResponseBody(nonce);
			response.setBody(dccResponseBody);
		}
		else
		{
			// Otherwise, decline the DCC request.
			response = buildResponse(603, "DECLINE", "application/x-msnmsgr-transrespbody", request);
			// Set the response body.
			const QString responseBody = QString("SessionID: %1\r\n\r\n").arg(identifier);
			response.setBody(responseBody);
		}

		// Set the response context information.
		response.setIdentifier(dialog->transactionId(true));
		response.setRelatesTo(request.identifier());

		// Send the status response message.
		send(response, 0, 0);
	}
}

bool SessionClient::isMyDirectConnectionSetupRequestLoser(Dialog *dialog, const SlpRequest& request)
{
	///////////////////////////////////////////////////////////////
	// NOTE Once a dialog has been established, any participant
	// can initiate a Direct Connection setup request.  For this
	// reason, it is required to check if an overlapping Direct
	// Connection setup request occurs.  This is done my conparing
	// the NetID fields in both request bodys.  The NetID which is
	// greater determines who wins and therefore who must discard
	// their request.
	///////////////////////////////////////////////////////////////

	bool isMyRequestLoser = true;
	Transaction *transaction = dialog->transactions().last();
	if (transaction->request().method() == QString::fromLatin1("INVITE") &&
		transaction->request().contentType() == QString::fromLatin1("application/x-msnmsgr-transreqbody"))
	{
		kdDebug() << k_funcinfo << "Got overlapping Direct Connection setup request" << endl;
		// Retrieve the peers Net ID.
		QRegExp regex("NetID: (\\-?\\d+)\r\n");
		regex.search(request.body());
		const Q_INT32 peerNetId = regex.cap(1).toInt();
		// Retrieve our Net ID.
		regex = QRegExp("NetID: (\\-?\\d+)\r\n");
		regex.search(transaction->request().body());
		const Q_INT32 myNetId = regex.cap(1).toInt();
		// Compare the Net IDs.
		isMyRequestLoser = (myNetId < peerNetId);
		if (isMyRequestLoser)
		{
			kdDebug() << k_funcinfo << "Responding to peer's request and dropping mine since peer's id is greater." << endl;
			endTransaction(transaction);
		}
		else
		{
			kdDebug() << k_funcinfo << "Dropping peer's request since my id is greater." << endl;
		}
	}

	return isMyRequestLoser;
}

const QString SessionClient::buildDirectConnectionSetupResponseBody(const QUuid& nonce)
{
	// TODO handle the hashed nonce field.
	Q_UNUSED(nonce);

	const QString CrLf = QString::fromLatin1("\r\n");

	QString responseBody;
	QTextStream stream(responseBody, IO_WriteOnly);
	Q_UINT16 port = 50050;
	// Try to get the transport to listen for incoming transport bridge connections.
	bool isListening = d->transport->listen(d->internalIpAddress , port);
	stream << QString::fromLatin1("Bridge: ") << QString::fromLatin1("TCPv1") << CrLf;
	stream << QString::fromLatin1("Listening: ") << (isListening ? "true" : "false") << CrLf;
	if (isListening)
	{
		stream << QString::fromLatin1("Nonce: ") << QUuid::createUuid().toString().upper() << CrLf;
		stream << QString::fromLatin1("IPv4External-Addrs: ") << d->externalIpAddress << CrLf;
		stream << QString::fromLatin1("IPv4External-Port: ") << QString::number(port) << CrLf;
		if (d->internalIpAddress != d->externalIpAddress)
		{
			stream << QString::fromLatin1("IPv4Internal-Addrs: ") << d->internalIpAddress << CrLf;
			stream << QString::fromLatin1("IPv4Internal-Port: ") << QString::number(port) << CrLf;
		}
	}
	else
	{
		stream << QString::fromLatin1("Nonce: ") << QUuid().toString().upper() << CrLf;
	}

	stream << CrLf;

	return responseBody;
}

void SessionClient::onDirectConnectionOfferRequest(const SlpRequest& request)
{
	///////////////////////////////////////////////////////////
	// NOTE A Direct Connection offer request is sent if
	// we responded to a Direct Connection setup request
	// and specified that we were not listening on any port.
	// For this reason, the peer offers to be the listening
	// endpoint and the received request contains the peer's
	// listening endpoint information.
	///////////////////////////////////////////////////////////

	// Determine if an existing dialog exists.
	Dialog *dialog = getDialogByCallId(QUuid(request.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Established)
	{
		kdDebug() << k_funcinfo << "Got Direct Connection offer INVITE request" << endl;

		// Send an acknowledge to the remote endpoint.
		sendAcknowledge(0, dialog->transactionId(true), request.identifier(), 0);

		kdDebug() << "************* BEGIN Direct Connection offer information *************" << endl;

		// Get the request body.
		const QString & requestBody = request.body();

		// Parse the direct connection description from the request body.

		// NOTE The bridge type will be the one that we selected
		// associated with the received DCC setup request.
		QRegExp regex("Bridge: ([a-zA-Z0-9 ]*)\r\n");
		regex.search(requestBody);
		const QString bridge = regex.cap(1);
		kdDebug() << k_funcinfo << "Got bridge, " << bridge << endl;

		const QString sessionId = regex.cap(1);
		regex = QRegExp("Listening: ([a-z]*)\r\n");
		regex.search(requestBody);
		// NOTE Indicates whether the peer is transport bridge
		// is listening.
		const bool isListening = (QString::fromLatin1("true") == regex.cap(1));
		kdDebug() << k_funcinfo << "Got bridge listening, " << regex.cap(1) << endl;

		regex = QRegExp("Nonce: \\{([0-9A-F\\-]*)\\}");
		regex.search(requestBody);
		QUuid nonce = QUuid(regex.cap(1));
		kdDebug() << k_funcinfo << "Got nonce, " << nonce.toString().upper() << endl;

		regex = QRegExp("IPv4External-Addrs: ([^\r\n]*)\r\n");
		regex.search(requestBody);
		const QString externalIpAddress = regex.cap(1);
		kdDebug() << k_funcinfo << "Got external ip address, " << externalIpAddress << endl;

		regex = QRegExp("IPv4External-Port: (\\d+)\r\n");
		regex.search(requestBody);
		const QString externalPort = regex.cap(1);
		kdDebug() << k_funcinfo << "Got external port, " << externalPort << endl;

		regex = QRegExp("IPv4Internal-Addrs: ([^\r\n]*)\r\n");
		regex.search(requestBody);
		const QString internalIpAddress = regex.cap(1);
		kdDebug() << k_funcinfo << "Got internal ip address, " << internalIpAddress << endl;

		regex = QRegExp("IPv4Internal-Port: (\\d+)\r\n");
		regex.search(requestBody);
		const QString internalPort = regex.cap(1);
		kdDebug() << k_funcinfo << "Got internal port, " << internalPort << endl;

		regex = QRegExp("SessionID: (\\d+)");
		regex.search(requestBody);
		QString identifier = regex.cap(1);
		if (identifier.isEmpty())
		{
			// The SessionID field was not sent in the response body.
			identifier = QString::number(dialog->session);
		}

		kdDebug() << "************* END Direct Connection offer information   *************" << endl;

		// Create the 200 OK response message
		SlpResponse response = buildResponse(200, "OK", "application/x-msnmsgr-transrespbody", request);
		// Set the response body.
		const QString responseBody = QString("SessionID: %1\r\n\r\n").arg(identifier);
		response.setBody(responseBody);
		// Set the response context information.
		response.setIdentifier(dialog->transactionId(true));
		response.setRelatesTo(request.identifier());

		// Send the 200 OK status response message.
		send(response, 0, 0);

		// TODO Connect to the address range and port specified.

		// TODO Update the session state information.
	}
}

bool SessionClient::supportsDirectConnectivity(const QString& connectionType, bool behindFirewall, bool uPnpSupported, bool sameNetwork)
{
	bool bSupportsDirectConnectivity = false;
	if (connectionType == QString::fromLatin1("Direct-Connect"))
	{
		bSupportsDirectConnectivity = !behindFirewall;
	}
	else
	if (connectionType == QString::fromLatin1("IP-Restrict-NAT"))
	{
		// IP addresses are different, but external and internal ports are the same.
		bSupportsDirectConnectivity = (!behindFirewall && uPnpSupported);
	}
	else
	if (connectionType == QString::fromLatin1("Symmetric-NAT"))
	{
		// IP addresses are different, and the external and internal ports are different.
		bSupportsDirectConnectivity = (!behindFirewall && uPnpSupported);// && !sameNetwork);
	}
	else
	if (connectionType == QString::fromLatin1("Unknown-NAT"))
	{
		bSupportsDirectConnectivity = (!behindFirewall && uPnpSupported);// && !sameNetwork);
	}


	return bSupportsDirectConnectivity;
}

const QString SessionClient::buildDirectConnectionSetupRequestBody()
{
	const QString CrLf = QString::fromLatin1("\r\n");
	QString requestBody;
	QTextStream stream(requestBody, IO_WriteOnly);

	stream << QString::fromLatin1("Bridges: ") << "TCPv1" << CrLf;
	stream << QString::fromLatin1("NetID: ") << htonl(QHostAddress(d->externalIpAddress).toIPv4Address()) << CrLf;
	stream << QString::fromLatin1("UPnPNat: ") << "false" << CrLf;
	stream << QString::fromLatin1("Conn-Type: ") << "Unknown-Connect" << CrLf;
	stream << QString::fromLatin1("ICF:  ") << "false" << CrLf;

	return requestBody;
}

bool SessionClient::parseSessionRequestBody(const QString& requestBody, QMap<QString, QVariant> & collection)
{
	bool parseError = false;
	QRegExp regex("EUF-GUID: \\{([0-9A-F\\-]*)\\}");
	// Check if the euf guid field is valid.
	if (regex.search(requestBody) == -1) {
		parseError = true;
	}
	collection.insert("EUF-GUID", regex.cap(1));

	regex = QRegExp("SessionID: (\\d+)");
	// Check if the session id field is valid.
	if (regex.search(requestBody) == -1) {
		parseError = true;
	}
	collection.insert("SessionID", regex.cap(1));

	regex = QRegExp("AppID: (\\d+)");
	// Check if the app id field is valid.
	if (regex.search(requestBody) == -1) {
		parseError = true;
	}
	collection.insert("AppID", regex.cap(1));

	regex = QRegExp("Context: ([0-9a-zA-Z+/=]*)");
	// Check if the context field is valid.
	if (regex.search(requestBody) == -1) {
		parseError = true;
	}
	collection.insert("Context", regex.cap(1));

	return parseError;
}

void SessionClient::onInitialInviteRequest(const SlpRequest& request)
{
	Dialog *dialog = getDialogByCallId(QUuid(request.headers()["Call-ID"].toString()));
	if (dialog != 0l)
	{
		kdDebug() << "Got duplicate initial INVITE request, dialog="
		<< dialog->identifier().toString().upper() << " -- ignoring it" << endl;
		return;
	}

	// Create the dialog from the INVITE request.
	dialog = new Dialog(new Transaction(request, false), this);
	// Add the pending dialog to the call map.
	addDialogToCallMap(dialog);

	// Send an acknowledge to the remote endpoint.
	sendAcknowledge(0, dialog->transactionId(true), request.identifier());

	// Get the request body.
	const QString & requestBody = request.body();

	QMap<QString, QVariant> sessionDescription;
	// Parse the session description from the request body.
	bool parseError = parseSessionRequestBody(requestBody, sessionDescription);
	if (parseError)
	{
		// If a parse error is encountered, inform the peer
		// by responding with an internal error.
		kdDebug() << k_funcinfo << "Got invalid session description on initial INVITE request -- responding with 500" << endl;

		// Generate a No Such Call response
		SlpResponse response = buildResponse(500, "Internal Error", "null", request);
		// Set the response context information.
		response.setIdentifier(dialog->transactionId(true));
		response.setRelatesTo(request.identifier());

		// Send the 500 Internal Error response message.
		send(response, 0);

		// Wait 30 seconds for an acknowledge before termining the dialog.
		// If a timeout occurs, terminate the dialog anyway.
		dialog->terminate(30000);
		return;
	}

	// Retrieve the session decription fields.
	const QUuid uuid = QUuid(sessionDescription["EUF-GUID"].toString());
	const Q_UINT32 sessionId = sessionDescription["SessionID"].toInt();
	const Q_UINT32 appId = sessionDescription["AppID"].toInt();
	const QString context = sessionDescription["Context"].toString();

	// Try to select the appropriate handler for the session
	// invitation based on uuid.
	if (d->applications.contains(uuid))
	{
		// If there is a registered application that can handle
		// the received session description (offer), pass the offer
		// to the application.
		Application *application = d->applications[uuid];
		// Connect the signal/slot.
		QObject::connect(application , SIGNAL(accept(const QString&)), this,
		SLOT(acceptSession(const QString&)));
		QObject::connect(application , SIGNAL(decline(const QString&)), this,
		SLOT(declineSession(const QString&)));

		dialog->session = sessionId;

		// Handle the session desciption (offer).
		application->handleRequest(sessionId, appId, context);
	}
	else
	{
		// Otherwise, decline the session.
		kdDebug() << "Got initial INVITE request for unsupported application" << endl;

		declineSession(dialog, sessionId);
	}
}

//END

//BEGIN UA Client Functions

//////////////////////////////////////////////////////////////////////
// Handles session layer protocol responses received
// Slp responses have to be initiated with a transaction (request).
//////////////////////////////////////////////////////////////////////
void SessionClient::onResponseMessage(const SlpResponse& response)
{
	// Try to match the received response to a locally
	// initiated transaction (request).
	if (!d->transactions.contains(getTransactionBranchFrom(response)))
	{
		// If no transaction is found, ignore the received
		// response message as the client did not initiate
		// the transaction with a sent request.
		kdDebug() << k_funcinfo << "Got a transactionless response -- ignoring" << endl;
		return;
	}

	// Get the status code of the response.
	const Q_INT32 statusCode = response.statusCode();
	if (statusCode == 200)
	{
		// NOTE MSNSLP/1.0 200 OK

		// Get the response content type.
		const QString contentType = response.contentType();
		if (contentType == QString::fromLatin1("application/x-msnmsgr-sessionreqbody"))
		{
			onSessionRequestAccepted(response);
		}
		else
		if (contentType == QString::fromLatin1("application/x-msnmsgr-transrespbody"))
		{
			onDirectConnectionRequestAccepted(response);
		}
	}
	else
	if (statusCode == 404)
	{
		// NOTE MSNSLP/1.0 404 Not Found
		onRecipientUriNotFound(response);
	}
	else
	if (statusCode == 481)
	{
		// NOTE MSNSLP/1.0 481 No Such Call
		onNoSuchCall(response);
	}
	else
	if (statusCode == 500)
	{
		// NOTE MSNSLP/1.0 500 Internal Error
		onInternalError(response);
	}
	else
	if (statusCode == 603)
	{
		// NOTE MSNSLP/1.0 603 Decline

		// Get the response content type.
		const QString contentType = response.contentType();
		if (contentType == QString::fromLatin1("application/x-msnmsgr-sessionreqbody"))
		{
			onSessionRequestDeclined(response);
		}
		else
		if (contentType == QString::fromLatin1("application/x-msnmsgr-transrespbody"))
		{
			onDirectConnectionRequestDeclined(response);
		}
	}
	else
	{
		kdDebug() << k_funcinfo << "Got unknown response status code, " << statusCode
		<< " -- dropping response" << endl;
	}
}

// TODO
bool SessionClient::parseDirectConnectionResponseBody(const QString& responseBody, QMap<QString, QVariant> & collection)
{
	Q_UNUSED(responseBody);
	Q_UNUSED(collection);

	return false;
}

void SessionClient::onDirectConnectionRequestAccepted(const SlpResponse& response)
{
	// Determine if an existing dialog exists.
	Dialog *dialog = getDialogByCallId(QUuid(response.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Established)
	{
		// Send an acknowledge to the remote endpoint.
		sendAcknowledge(0, dialog->transactionId(true), response.identifier(), 0);

		// Get the response body.
		const QString responseBody = response.body();

		/////////////////////////////////////////////////////////////////
		// Retrieve the last transaction associated with this response
		// to determine if the request was a Direct Connection setup or
		// a Direct Connection offer request.
		/////////////////////////////////////////////////////////////////

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		Transaction *transaction = d->transactions[branch];
		const SlpRequest & request = transaction->request();
		if (request.contentType() == QString::fromLatin1("application/x-msnmsgr-transreqbody"))
		{
			kdDebug() << k_funcinfo << "Got Direct Connection setup request accepted" << endl;

			kdDebug() << "************* BEGIN Direct Connection setup information *************" << endl;

			// Parse the direct connection description from the response body.
			// NOTE The bridge type will be the one that we offered
			// associated with the received Direct Connection setup request.
			QRegExp regex("Bridge: ([a-zA-Z0-9 ]*)\r\n");
			regex.search(responseBody);
			const QString bridge = regex.cap(1);
			kdDebug() << k_funcinfo << "Got bridge, " << bridge << endl;

			const QString sessionId = regex.cap(1);
			regex = QRegExp("Listening: ([a-z]*)\r\n");
			regex.search(responseBody);
			// NOTE Indicates whether the peer's transport bridge is listening.
			const bool isListening = (QString::fromLatin1("true") == regex.cap(1));
			kdDebug() << k_funcinfo << "Got bridge listening, " << regex.cap(1) << endl;

			regex = QRegExp("Nonce: \\{([0-9A-F\\-]*)\\}");
			regex.search(responseBody);
			const QUuid nonce = QUuid(regex.cap(1));
			kdDebug() << k_funcinfo << "Got nonce, " << nonce.toString().upper() << endl;

			regex = QRegExp("IPv4External-Addrs: ([^\r\n]*)\r\n");
			regex.search(responseBody);
			const QString externalIpAddress = regex.cap(1);
			kdDebug() << k_funcinfo << "Got external ip address, " << externalIpAddress << endl;

			regex = QRegExp("IPv4External-Port: (\\d+)\r\n");
			regex.search(responseBody);
			const QString externalPort = regex.cap(1);
			kdDebug() << k_funcinfo << "Got external port, " << externalPort << endl;

			regex = QRegExp("IPv4Internal-Addrs: ([^\r\n]*)\r\n");
			regex.search(responseBody);
			const QString internalIpAddress = regex.cap(1);
			kdDebug() << k_funcinfo << "Got internal ip address, " << internalIpAddress << endl;

			regex = QRegExp("IPv4Internal-Port: (\\d+)\r\n");
			regex.search(responseBody);
			const QString internalPort = regex.cap(1);
			kdDebug() << k_funcinfo << "Got internal port, " << internalPort << endl;

			kdDebug() << "************* END Direct Connection setup information   *************" << endl;

			// TODO Connect to the address range and port specified.

			// TODO Update the session state information.
		}
		else
		if (request.contentType() == QString::fromLatin1("application/x-msnmsgr-transrespbody"))
		{
			kdDebug() << k_funcinfo << "Got Direct Connection offer request accepted" << endl;

			QRegExp regex("SessionID: (\\d+)");
			regex.search(responseBody);
			const QString identifier = regex.cap(1);

			// TODO Update the session state information.
		}

		// End the transaction.
		endTransaction(transaction);
	}
}

void SessionClient::onDirectConnectionRequestDeclined(const SlpResponse& response)
{
	// Determine if an existing dialog exists.
	Dialog *dialog = getDialogByCallId(QUuid(response.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Established)
	{
		kdDebug() << k_funcinfo << "Got Direct Connection request declined" << endl;

		// Send an acknowledge to the remote endpoint.
		sendAcknowledge(0, dialog->transactionId(true), response.identifier(), 0);

		// Get the response body.
		const QString responseBody = response.body();

		QRegExp regex("SessionID: (\\d+)");
		regex.search(responseBody);
		const QString identifier = regex.cap(1);

		// TODO Update the session state information.

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		// End the transaction.
		Transaction *transaction = d->transactions[branch];
		endTransaction(transaction);
	}
}

void SessionClient::onInternalError(const SlpResponse& response)
{
	// Try to retrieve the dialog associated with this response.
	Dialog *dialog = getDialogByCallId(QUuid(response.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Established)
	{
		// If the dialog exists, terminate the dialog and the transaction
		// associated with the received response.
		kdDebug() << k_funcinfo << "Got Internal Error, dialog="
		<< dialog->identifier().toString().upper() << " -- terminating" << endl;

		// Send an acknowledge to the remote endpoint
		sendAcknowledge(0, dialog->transactionId(true), response.identifier());

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		// End the transaction since an error has occured.
		Transaction *transaction = d->transactions[branch];
		endTransaction(transaction);

		// Terminate the dialog and its associated session(s)
		dialog->terminate();
	}
}

void SessionClient::onNoSuchCall(const SlpResponse& response)
{
	kdDebug() << k_funcinfo << "called" << endl;
	// Determine if an existing dialog exists.  If one exists, terminate the dialog.
	Dialog *dialog = getDialogByCallId(QUuid(response.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Established)
	{
		kdDebug() << k_funcinfo << "Got No Such Call, dialog="
		<< dialog->identifier().toString().upper() << " -- terminating" << endl;

		// Send an acknowledge to the remote endpoint
		sendAcknowledge(0, dialog->transactionId(true), response.identifier());

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		// End the transaction since an error has occured.
		Transaction *transaction = d->transactions[branch];
		endTransaction(transaction);

		// Terminate the dialog and its associated session(s)
		dialog->terminate();
	}
}

void SessionClient::onRecipientUriNotFound(const SlpResponse& response)
{
	// Try to retrieve the dialog associated with this response.
	Dialog *dialog = getDialogByCallId(QUuid(response.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Established)
	{
		// If the dialog exists, terminate the dialog and the transaction
		// associated with the received response.
		kdDebug() << k_funcinfo << "Got recipient Uri Not Found, dialog="
		<< dialog->identifier().toString().upper() << " -- terminating" << endl;

		// Send an acknowledge to the remote endpoint
		sendAcknowledge(0, dialog->transactionId(true), response.identifier());

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		// End the transaction since an error has occured.
		Transaction *transaction = d->transactions[branch];
		endTransaction(transaction);

		// Terminate the dialog and its associated session(s)
		dialog->terminate();
	};
}

void SessionClient::onSessionRequestAccepted(const SlpResponse& response)
{
	// Determine if a pending dialog exists.
	Dialog *dialog = getDialogByCallId(QUuid(response.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Pending)
	{
		// If so, set the dialog state to established.
		dialog->establish();

		// Send an acknowledge to the other side.
		sendAcknowledge(0, dialog->transactionId(true), response.identifier());

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		// End the transaction.
		Transaction *transaction = d->transactions[branch];
		endTransaction(transaction);
	}
}

void SessionClient::onSessionRequestDeclined(const SlpResponse& response)
{
	// Determine if a pending dialog associated with this response exists.
	Dialog *dialog = getDialogByCallId(QUuid(response.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Pending)
	{
		// If so, terminate the dialog since the INVITE request
		// to establish the dialog was declined.
		kdDebug() << k_funcinfo << "Dialog DECLINED, " << dialog->identifier().toString().upper() << endl;

		// Send an acknowledge to the other side.
		sendAcknowledge(0, dialog->transactionId(true), response.identifier());

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		Transaction *transaction = d->transactions[branch];
		// Check if the transaction associated with the response
		// was a dialog creating INVITE request.
		if (d->transactions[branch]->request().method() == QString::fromLatin1("INVITE"))
		{
			// If so, end the transaction.
			endTransaction(transaction);
		}

		// Terminate the pending dialog.
		dialog->terminate();
	}
}

//END

//BEGIN Session Functions

//END

void SessionClient::sendAcknowledge(const Q_INT32 destination, const Q_INT32 identifier, const Q_INT32 relatesTo, const Q_UINT32 priority)
{
	kdDebug() << k_funcinfo << "SENDING ACK message" << endl;
	d->transport->sendAcknowledge(destination, identifier, relatesTo, priority);
}

void SessionClient::send(const SlpMessage& message, const Q_UINT32 destination, const Q_UINT32 priority)
{
	QByteArray bytes;
	QTextStream stream(bytes, IO_WriteOnly);

	const QString CrLf = QString::fromLatin1("\r\n");
	// Write the start line to the stream
	stream << message.startLine() << CrLf;
	// Write the headers to the stream.
	stream << "To: " << message.headers()["To"].toString() << CrLf;
	stream << "From: " << message.headers()["From"].toString() << CrLf;
	stream << "Via: " << message.headers()["Via"].toString() << CrLf;
	stream << "CSeq: " << message.headers()["CSeq"].toString() << CrLf;
	stream << "Call-ID: " << message.headers()["Call-ID"].toString() << CrLf;
	stream << "Max-Forwards: " << message.headers()["Max-Forwards"].toString() << CrLf;
	stream << "Content-Type: " << message.headers()["Content-Type"].toString() << CrLf;

	// Get the message content length.
	Q_INT32 contentLength = message.body().length();
	contentLength = contentLength ? contentLength + 1 : 0;
	// Write the message content length to the message buffer.
	stream << QString::fromLatin1("Content-Length: ") << contentLength << CrLf
	<< CrLf;

	if (contentLength > 0)
	{
		// If the message has a body, write the body content to the stream.
		stream << message.body() << '\0';
	}

	kdDebug() << "============= SENDING message=" << message.identifier() << ", " << QString(bytes) << endl;

	// Send the message data bytes via the transport layer.
	d->transport->sendDatagram(bytes, destination, message.identifier(), message.relatesTo(), priority);
}

//BEGIN GIF Message Functions

//////////////////////////////////////////////////////////////////////
// Handles session layer protocol image data messages received
// Supported types: image/gif.
//////////////////////////////////////////////////////////////////////

void SessionClient::onImageReceived(const QByteArray& content, const Q_INT32 identifier, const Q_INT32 relatesTo)
{
	Q_UNUSED(relatesTo);

	// NOTE The entire content is in UTF16 (unicode) encoding.
	QTextStream sr(content, IO_ReadOnly);
	sr.setEncoding(QTextStream::RawUnicode);
	QString message = sr.read();

	// Try to retrieve the image/gif data.
	QRegExp regex("base64:([0-9a-zA-Z+/=]*)");
	if (regex.search(message) != -1)
	{
		QByteArray bytes;
		// Convert from base64 encoded binary to a byte array
		KCodecs::base64Decode(regex.cap(1).utf8(), bytes);

		//
		// Write the image to a temporary file and fire
		// the imageReceived event.
		//
		KTempFile *temporaryFile = new KTempFile(locateLocal("tmp", "image-gif-"), ".gif");
		QDataStream *stream = temporaryFile->dataStream();
		if (stream)
		{
			// Write the image/gif data to the temporary file.
			stream->writeRawBytes(bytes.data(), bytes.size());
		}
		temporaryFile->close();

		emit imageReceived(temporaryFile);
		temporaryFile = 0l;
	}

	// Send an acknowledge to the other side.
	sendAcknowledge(64, identifier, 0x00048391);
}

void SessionClient::sendImage(const QString& path)
{
	QFile file(path);
	if (!file.open(IO_ReadOnly))
	{
		kdDebug() << k_funcinfo << "Could not open image file to send, " << path << endl;
		return;
	}

	// Convert the image data to base64 binary data.
	QByteArray base64Binary = KCodecs::base64Encode(file.readAll());

	QByteArray bytes;
	QTextStream stream(bytes, IO_WriteOnly);
	stream.setEncoding(QTextStream::RawUnicode);
	stream << "MIME-Version: 1.0" << "\r\n"
	<< "Content-Type: image/gif" << "\r\n"
	<< "\r\n"
	<< "base64:" << base64Binary.data()
	<< '\0';

	d->transport->sendDatagram(bytes, 64, 0x00048390, 0);
}

//END

}

#include "sessionclient.moc"
