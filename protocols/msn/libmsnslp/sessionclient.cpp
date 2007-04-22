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
#include "dialog.h"
#include "filetransfersession.h"
#include "msnobjectsession.h"
#include "sessionnotifier.h"
#include "transaction.h"
#include "transport.h"
#include "kopetecontact.h"

#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qhostaddress.h>
#include <qmetaobject.h>
#include <qptrlist.h>
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
		bool behindFirewall;
		QString connectionType;
		QMap<QUuid, Dialog*> dialogs;
		QString externalIpAddress;
		QString internalIpAddress;
		Kopete::Contact *me;
		QString myUri;
		Q_INT32 netId;
		Kopete::Contact *peer;
		QString peerUri;
		Q_UINT32 nextSessionId;
		QMap<Q_INT32, Session*> sessions;
		QString supportedBridgeTypes;
		QMap<QUuid, Transaction*> transactions;
		Transport* transport;
		bool uPnpNAT;
		QString version;
};

SessionClient::SessionClient(const QMap<QString, QVariant> & properties, Kopete::Contact *me, Kopete::Contact *peer, Transport* transport, QObject *parent) : QObject(parent), d(new SessionClientPrivate())
{
	// Configure the client using the supplied properties
	d->behindFirewall = false;
	d->externalIpAddress = properties["externalIpAddress"].toString();
	d->internalIpAddress = properties["internalIpAddress"].toString();

	d->me = me;
	d->myUri = me->contactId();
	d->peer = peer;
	d->peerUri = peer->contactId();
	// Set the transport bridge types supported by the client.
	d->supportedBridgeTypes = QString::fromLatin1("TCPv1");
	d->uPnpNAT = true;//properties["uPnpNAT"].toBool();
	if (d->uPnpNAT)
	{
		d->connectionType = QString::fromLatin1("Unknown-NAT");
		QHostAddress address;
		address.setAddress(d->externalIpAddress);
		const Q_INT32 netId = address.toIPv4Address();
		d->netId = (Q_INT32)htonl(netId);
	}
	else
	{
		d->connectionType = QString::fromLatin1("Unknown-Connect");
		d->netId = 0;
	}

	// Set the client/protocol version.
	d->version = QString::fromLatin1("1.0");

	// Set the transport for the client.
	d->transport = transport;
	d->nextSessionId = (Q_INT32)(((double)(rand() * 1.0/0x7FFFFFFF))*(0xA235 - 0x2DCC)) + 0x2DCC;
	// Initialize the client.
	initialize();
}

SessionClient::~SessionClient()
{
	delete d;
}

void SessionClient::initialize()
{
	SessionNotifier *notifier = new SessionNotifier(0, SessionNotifier::Normal, this);
	// Connect the signal/slot
	QObject::connect(notifier, SIGNAL(messageReceived(const QByteArray&, const Q_INT32, const Q_INT32)), this,
	SLOT(onReceived(const QByteArray&, const Q_INT32, const Q_INT32)));
	QObject::connect(notifier, SIGNAL(messageAcknowledged(const Q_INT32)), this,
	SLOT(onSend(const Q_INT32)));
	// Register the notifier for the session client , session 0
	d->transport->registerPort(notifier->session(), notifier);

	notifier = new SessionNotifier(64, SessionNotifier::Normal, this);
	// Connect the signal/slot
	QObject::connect(notifier, SIGNAL(messageReceived(const QByteArray&, const Q_INT32, const Q_INT32)), this,
	SLOT(onImageReceived(const QByteArray&, const Q_INT32, const Q_INT32)));
	// Register the notifier for the session client , session 64
	d->transport->registerPort(notifier->session(), notifier);

	notifier = 0l;
}

bool SessionClient::isActive() const
{
	kdDebug() << k_funcinfo << "Active dialogs, " << d->dialogs.size() << endl;
	return (d->dialogs.size() != 0);
}

Q_UINT32 SessionClient::nextSessionId() const
{
	return (++d->nextSessionId);
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

void SessionClient::createSessionInternal(const QUuid& uuid, const Q_UINT32 sessionId, const Q_UINT32 appId, const QString& context)
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
	request.setId(0);
	request.setCorrelationId(rand() & 0xEC253);

	// Add the pending dialog to the call map.
	addDialogToCallMap(dialog);

	// Create a new transaction for the request.
	Transaction *transaction = new Transaction(request, true);
	dialog->setInitialTransaction(transaction);

	// Begin the transaction.
	beginTransaction(transaction);

	// Send the transaction request.
	send(transaction->request(), 0, 0);
}

//BEGIN Application Functions

void SessionClient::requestObject(const QString& object)
{
	MsnObjectSession *session = new MsnObjectSession(object, nextSessionId(), Session::Incoming, this);
	// Connect the signal/slot
	QObject::connect(session, SIGNAL(objectReceived(const QString&, KTempFile*)), this,
	SIGNAL(objectReceived(const QString&, KTempFile*)));

	SessionNotifier *notifier = new SessionNotifier(session->id(), SessionNotifier::Object, this);
	// Connect the signal/slot
	QObject::connect(notifier, SIGNAL(messageReceived(const QByteArray&, const Q_INT32, const Q_INT32)), session,
	SLOT(onReceive(const QByteArray&, const Q_INT32, const Q_INT32)));
	QObject::connect(notifier, SIGNAL(messageAcknowledged(const Q_INT32)), session,
	SLOT(onSend(const Q_INT32)));
	QObject::connect(notifier, SIGNAL(dataReceived(const QByteArray&, bool)), session,
 	SLOT(onDataReceived(const QByteArray&, bool)));

	// Register the notifier for the session
	d->transport->registerPort(notifier->session(), notifier);

	// Add the session to the list of session.
	d->sessions.insert(session->id(), session);

	// Get the EUF GUID that uniquely identifies the session type.
	QUuid eufguid = QUuid(MsnObjectSession::staticMetaObject()->classInfo("EUF-GUID"));
	// Create the context field of the session description.
	const QString context = QString::fromUtf8(KCodecs::base64Encode(object.utf8()));

	// Create the session to request the msn object.
	createSessionInternal(eufguid, session->id(), 12, context);
}

void SessionClient::sendFile(const QString& path)
{
	kdDebug() << k_funcinfo << path << endl;
	FileTransferSession *session = new FileTransferSession(nextSessionId(), Session::Outgoing, d->me, this);
	// Connect the signal/slot
	QObject::connect(session, SIGNAL(sendFile(QFile*)), this,
	SLOT(onSessionSendFile(QFile*)));

	QFile *file = new QFile(path);
	session->setDataStore(file);

	SessionNotifier *notifier = new SessionNotifier(session->id(), SessionNotifier::FileTransfer, this);
	// Connect the signal/slot
	QObject::connect(notifier, SIGNAL(messageAcknowledged(const Q_INT32)), session,
	SLOT(onSend(const Q_INT32)));

	// Register the notifier for the session
	d->transport->registerPort(notifier->session(), notifier);

	// Add the session to the list of session.
	d->sessions.insert(session->id(), session);

	// Create the file information section of the context field.
	QByteArray info(638);
	info.fill('\0');

	QDataStream stream(info, IO_WriteOnly);
	stream.setByteOrder(QDataStream::LittleEndian);
	// Write the header length to the stream.
	stream << (Q_UINT32)info.size();
	// Write client version to the stream.
	stream << (Q_UINT32)3;
	// Write the file size to the stream.
	stream << (Q_UINT64)file->size();
	// TODO support file preview. For now disable file preview.
	// Write the file type to the stream.
	stream << (Q_UINT32)1;
	// Get the name of the file to be sent.
	const QString name = path.section('/', -1);
	// Write the file name in UTF16 (unicode) encoding to the stream.
	stream.writeRawBytes((char*)((void*)name.ucs2()), (name.length()*2) + 2);
	// NOTE Background Sharing base64 [540..569]
	// TODO add support for background sharing.

	// NOTE File - 0xFFFFFFFF
	// NOTE Background Sharing - 0xFFFFFFFE
	stream.device()->at(570);
	// Write file exchange type to the stream.
	stream << (Q_UINT32)0xFFFFFFFF;

	// Get the EUF GUID that uniquely identifies the session type.
	QUuid eufguid = QUuid(FileTransferSession::staticMetaObject()->classInfo("EUF-GUID"));
	// Create the context field of the session description.
	const QString context = QString::fromUtf8(KCodecs::base64Encode(info));

	// Create the session to request the file transfer.
	createSessionInternal(eufguid, session->id(), 2, context);
}

//END

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

//END

void SessionClient::onReceived(const QByteArray& content, const Q_INT32 id, const Q_INT32 correlationId)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	const QString message = QString::fromUtf8(content);
	kdDebug() << k_funcinfo << message << endl;

	// Try to parse the session layer protocol (slp)
	// message from the raw data received.
	const QString startLine = message.section("\r\n", 0, 0);
	if (startLine.startsWith("INVITE") || startLine.startsWith("BYE"))
	{
		// If the start line pattern matchs that of a request,
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
			Message::parseHeaders(message.section("\r\n\r\n", 0, 0), headers);
			// Copy the parsed headers to the request's header collection.
			request.copyHeadersFrom(headers);

			if (request.contentLength() > 0)
			{
				const QString requestBody = message.section("\r\n\r\n", 1, 1, QString::SectionIncludeTrailingSep);
				// Set the request body content.kdDebug() << k_funcinfo << "enter" << endl;
				request.setBody(requestBody);
			}

			// Set the message context information.
			request.setId(id);
			request.setCorrelationId(correlationId);

			// Handle the received slp request.
			onRequestMessage(request);
		}
	}
	else
	if (startLine.startsWith("MSNSLP"))
	{
		// If the start line pattern matchs that of a response,
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
			Message::parseHeaders(message.section("\r\n\r\n", 0, 0), headers);
			// Copy the parsed headers to the response's header collection.
			response.copyHeadersFrom(headers);

			if (response.contentLength() > 0)
			{
				const QString responseBody = message.section("\r\n\r\n", 1, 1, QString::SectionIncludeTrailingSep);
				// Set the response body content.
				response.setBody(responseBody);
			}

			// Set the message context information.
			response.setId(id);
			response.setCorrelationId(correlationId);

			// Handle the received slp response.
			onResponseMessage(response);
		}
	}
	else
	{
		kdDebug() << k_funcinfo << "Unrecognized message, id = " << id
		<< " -- ignoring" << endl;
	}

	kdDebug() << k_funcinfo << "leave" << endl;
}

void SessionClient::onSend(const Q_INT32 id)
{
	kdDebug() << k_funcinfo << "enter" << endl;
	bool isAcknowledgeForRequest = false;

	// Check whether we received an ACK to a sent request.
	QMap<QUuid, Transaction*>::Iterator it = d->transactions.begin();
	while (it != d->transactions.end())
	{
		if (it.data()->request().id() == id)
		{
			Transaction *transaction = it.data();
			// Mark the transaction as confirmed by the peer endpoint.
			transaction->confirm();
			kdDebug() << "Transaction COMFIRMED, branch = " << transaction->branch().toString().upper() << endl;

			if (transaction->request().method() == QString::fromLatin1("BYE"))
			{
				// Finish the transaction.
				endTransaction(transaction);

				// Try to retrieve a terminating dialog associated with the BYE transaction.
				Dialog *dialog = getDialogByTransactionId(transaction->request().id());
				if (dialog != 0l && dialog->state() == Dialog::Terminating)
				{
					// If found, terminate the dialog.
					dialog->terminate();
				}
			}

			isAcknowledgeForRequest = true;
			break;
		}
		it++;
	}

	if (!isAcknowledgeForRequest)
	{
		// Otherwise, check whether we received an ACK to a sent response.
		Dialog *dialog = getDialogByTransactionId(id);
		if (dialog != 0l && dialog->state() == Dialog::Pending)
		{
			// This ACK response, established the pending dialog.
			// So, set the dialog to established.
			dialog->establish();
		}
	}

	kdDebug() << k_funcinfo << "leave" << endl;
}

//BEGIN Dialog Handling Functions

void SessionClient::addDialogToCallMap(Dialog *dialog)
{
	if (dialog != 0l && !d->dialogs.contains(dialog->callId()))
	{
		const QUuid callId = dialog->callId();
		kdDebug() << k_funcinfo << "Adding dialog to call map. call id = "
		<< callId.toString().upper() << endl;

		// Connect the signal/slot.
		QObject::connect(dialog, SIGNAL(established()), this,
		SLOT(onDialogEstablish()));
		QObject::connect(dialog, SIGNAL(terminated()), this,
		SLOT(onDialogTerminate()));

		// Add the new dialog to the call map.
		d->dialogs.insert(callId, dialog);
	}
}

Dialog* SessionClient::getDialogByCallId(const QUuid& callId)
{
	Dialog *dialog = 0l;
	if (d->dialogs.contains(callId)) dialog = d->dialogs[callId];
	return dialog;
}

Dialog* SessionClient::getDialogBySessionId(const Q_UINT32 sessionId)
{
	Dialog *dialog = 0l;
	QMap<QUuid, Dialog*>::Iterator it = d->dialogs.begin();
	// Iterate through the registered dialogs.
	while (it != d->dialogs.end())
	{
		if (it.data()->session == sessionId)
		{
			// If the session associated with the current
			// dialog has a session id which matchs that
			// of supplied session id, then the dialog
			// has been found.
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
	// Iterate through the registered dialogs.
	while (it != d->dialogs.end())
	{
		if (it.data()->transactionId() == transactionId)
		{
			// If the transaction id of the current dialog
			// matchs that of supplied transaction id,
			// then the dialog has been found.
			dialog = it.data();
			break;
		}
		it++;
	}

	return dialog;
}

///////////////////////////////////////////////////////////////////////////
// Handles the establishment of a dialog after the acceptance of a locally
// or remotely initiated session request.
///////////////////////////////////////////////////////////////////////////
void SessionClient::onDialogEstablish()
{
	Dialog *dialog = static_cast<Dialog*>(const_cast<QObject*>(sender()));
	if (dialog != 0l)
	{
		kdDebug() << "Dialog ESTABLISHED, call id = " << dialog->callId().toString().upper() << endl;

		// Request a direct connection because we want to
		// transfer the session data efficiently.
// 		requestDirectConnection(dialog->session);

		// TODO Update the session state information.

		// Try to retrieve the session associated with the dialog.
		Session *session = d->sessions[dialog->session];
		if (session != 0l)
		{
			// Connect the signal/slot.
			QObject::connect(session, SIGNAL(cancelled()), this,
			SLOT(onSessionCancel()));
			QObject::connect(session, SIGNAL(ended()), this,
			SLOT(onSessionEnd()));
			QObject::connect(session, SIGNAL(sendMessage(const QByteArray&)), this,
			SLOT(onSessionSendMessage(const QByteArray&)));
			QObject::connect(session, SIGNAL(sendData(const QByteArray&)), this,
			SLOT(onSessionSendData(const QByteArray&)));

			// Start the session.
			session->start();
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// Handles the termination of a dialog after receiving a BYE request or
// an error has occurred.
///////////////////////////////////////////////////////////////////////////
void SessionClient::onDialogTerminate()
{
	Dialog *dialog = static_cast<Dialog*>(const_cast<QObject*>(sender()));
	if (dialog != 0l)
	{
		kdDebug() << "Dialog TERMINATED, call id = " << dialog->callId().toString().upper() << endl;

		// Disconnect from the signal/slot
		QObject::disconnect(dialog, 0, this, 0);

		const Q_UINT32 sessionId = dialog->session;

		Session *session = d->sessions[sessionId];
		// End the session if not already terminated.
		if (session != 0l)
		{
			if (session->state() != Session::Terminated)
			{
				// End the session.
				session->end();
			}

			// Disconnect from the signal/slot
			QObject::disconnect(session, 0, this, 0);
			// Remove the session from the list of sessions.
			d->sessions.remove(sessionId);
			// Unregister the notifier for the session.
			d->transport->unregisterPort(sessionId);

			// Delete the session as its lifetime has ended.
			session->deleteLater();
			session = 0l;
		}

		// Remove the dialog from the call map.
		removeDialogFromCallMap(dialog->callId());

		// Terminate all pending transactions.
		QPtrListIterator<Transaction> it(dialog->transactions());
		Transaction *transaction;
		while ((transaction = it.current()) != 0l)
		{
			if (transaction->isLocal() && transaction->state() != Transaction::Terminated)
			{
				endTransaction(transaction);
			}
			++it;
		}

		// Delete the dialog as its lifetime has ended.
		dialog->deleteLater();
		dialog = 0l;
	}
}

void SessionClient::removeDialogFromCallMap(const QUuid& callId)
{
	if (d->dialogs.contains(callId))
	{
		kdDebug() << k_funcinfo << "Removing dialog from call map. call id = " << callId.toString().upper() << endl;
		// Remove the dialog from the call map.
		d->dialogs.remove(callId);
	}
}

//END

//BEGIN Transaction Handling Functions

void SessionClient::beginTransaction(Transaction *transaction)
{
	if (transaction != 0l && !d->transactions.contains(transaction->branch()))
	{
		kdDebug() << "Transaction BEGIN, branch = " << transaction->branch().toString().upper() << endl;

		// Add the transaction to the list of active transactions.
		d->transactions.insert(transaction->branch(), transaction);
		// Connect the signal/slot
		QObject::connect(transaction, SIGNAL(timeout()), this,
		SLOT(onTransactionTimeout()));
		// Begin the transaction.
		transaction->begin();
	}
}

void SessionClient::endTransaction(Transaction *transaction)
{
	if (transaction != 0l)
	{
		const QUuid& branch = transaction->branch();
		if (d->transactions.contains(branch))
		{
			kdDebug() << "Transaction END, branch = " << branch.toString().upper() << endl;

			// Disconnect the signal/slot
			QObject::disconnect(transaction, SIGNAL(timeout()), this,
			SLOT(onTransactionTimeout()));
			// End the transaction.
			transaction->end();
			// Remove the transaction from the list of active transactions.
			d->transactions.remove(branch);
		}
	}
}

void SessionClient::onTransactionTimeout()
{
	Transaction *transaction = dynamic_cast<Transaction*>(const_cast<QObject*>(sender()));
	if (transaction != 0l)
	{
		kdDebug() << k_funcinfo << "Transaction TIMED OUT, branch = " << transaction->branch().toString().upper() << endl;
		// End the transaction.
		endTransaction(transaction);

		// Try to retrieve the dialog associated with the transaction.
		Dialog *dialog = getDialogByTransactionId(transaction->request().id());
		if (dialog != 0l && (dialog->state() == Dialog::Pending || dialog->state() == Dialog::Terminating))
		{
			// Terminate the dialog.
			dialog->terminate();
		}
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
	headers["Call-ID"] = dialog->callId().toString().upper();
	headers["Max-Forwards"] = 0;
	headers["Content-Type"] = contentType;
	headers["Content-Length"] = 0;

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
	headers["Content-Length"] = 0;

	return response;
}

void SessionClient::acceptSession(const Q_UINT32 sessionId)
{
	Dialog *dialog = getDialogBySessionId(sessionId);
	if (dialog == 0l)
		return;

	// Get the initial INVITE request.
	const SlpRequest & request = dialog->initialTransaction()->request();
	// Generate a 200 OK response
	SlpResponse response = buildResponse(SlpResponse::OK, "OK", "application/x-msnmsgr-sessionreqbody", request);
	// Create the response body.
	const QString responseBody = QString("SessionID: %1\r\n\r\n").arg(sessionId);
	// Set the response body.
	response.setBody(responseBody);
	// Set the response context information.
	response.setId(0);
	response.setCorrelationId(request.id());

	// Send the 200 OK status response message.
	send(response, 0);
}

void SessionClient::declineSession(const Q_UINT32 sessionId)
{
	Dialog *dialog = getDialogBySessionId(sessionId);
	if (dialog == 0l)
		return;

	// Get the initial INVITE request.
	const SlpRequest & request = dialog->initialTransaction()->request();

	// Generate a 603 DECLINE response
	SlpResponse response = buildResponse(SlpResponse::Decline, "DECLINE", "application/x-msnmsgr-sessionreqbody", request);
	// Create the response body.
	const QString responseBody = QString("SessionID: %1\r\n\r\n").arg(sessionId);
	// Set the response body.
	response.setBody(responseBody);
	// Set the response context information.
	response.setId(0);
	response.setCorrelationId(request.id());

	// Send the 603 DECLINE status response message.
	send(response, 0);

	dialog->terminate(30000);
}

void SessionClient::closeSessionInternal(const Q_UINT32 sessionId)
{
	Dialog *dialog = getDialogBySessionId(sessionId);
	if (dialog == 0l)
		return;

	// Set the dialog's state to terminating.
	dialog->setState(Dialog::Terminating);

	// Build the BYE request.
	SlpRequest request = buildRequest("BYE", "application/x-msnmsgr-sessionclosebody", dialog);
	// Create the request body.
	const QString requestBody = QString("SessionID: %1\r\n\r\n").arg(sessionId);
	// Set the request body.
	request.setBody(requestBody);
	//Set the request context information.
	request.setId(0);
	request.setCorrelationId(rand() % 0xCF54);

	// Create a new transaction for the request.
	Transaction *transaction = new Transaction(request, true);
	dialog->transactions().append(transaction);
	// Begin the transaction.
	beginTransaction(transaction);

	// Send the transaction request.
	send(transaction->request(), 0);
}

void SessionClient::requestDirectConnection(const Q_UINT32 sessionId)
{
	Dialog *dialog = getDialogBySessionId(sessionId);
	if (dialog == 0l)
		return;

	kdDebug() << k_funcinfo << "Requesting Direct Connection for session " << sessionId << endl;

	// Build the INVITE request.
	SlpRequest request = buildRequest("INVITE", "application/x-msnmsgr-transreqbody", dialog);
	// Build the Direct Connection setup body.
	const QString requestBody = buildDirectConnectionSetupRequestBody(sessionId);
	// Set the request body.
	request.setBody(requestBody);
	//Set the request context information.
	request.setId(0);
	request.setCorrelationId(rand() % 0xCF60);

	// Create a new transaction for the request.
	Transaction *transaction = new Transaction(request, true);
	dialog->transactions().append(transaction);
	// Begin the transaction.
	beginTransaction(transaction);

	// Send the transaction request.
	send(transaction->request(), 0, 0);
}

//BEGIN Request Handling Functions

//////////////////////////////////////////////////////////////////////
// Handles session layer protocol requests received
// Slp only supports the INVITE and BYE methods.
//////////////////////////////////////////////////////////////////////
void SessionClient::onRequestMessage(const SlpRequest& request)
{
	if (request.to() != d->myUri)
	{
		kdDebug() << k_funcinfo << "Got request which is not for me -- responding with 404" << endl;

		// Generate a Not Found response
		SlpResponse response = buildResponse(SlpResponse::NotFound, "Not Found", "null", request);
		// Set the response context information.
		response.setId(0);
		response.setCorrelationId(request.id());

		// Send the 404 Not Found status response message.
		send(response, 0);
		return;
	}

	if (request.method() == QString::fromLatin1("INVITE"))
	{
		// Get the content type of the request.
		const QString contentType = request.contentType();
		// NOTE INVITE MSNMSGR:muser MSNSLP/1.0
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

			// Generate an Internal Error response
			SlpResponse response = buildResponse(SlpResponse::InternalError, "Internal Error", "null", request);
			// Set the response context information.
			response.setId(0);
			response.setCorrelationId(request.id());

			// Send the 500 Internal Error status response message.
			send(response, 0);
		}
	}
	else
	if (request.method() == QString::fromLatin1("BYE"))
	{
		// NOTE BYE MSNMSGR:muser MSNSLP/1.0

		// Handle the bye request.
		onByeRequest(request);
	}
}

bool SessionClient::parseSessionCloseBody(const QString& requestBody, QMap<QString, QVariant> & parameters)
{
	bool parseError = false;
	QRegExp regex("Context: ([0-9a-zA-Z+/=]*)");
	// Check if the context field is valid.
	if (regex.search(requestBody) != -1)
	{
		QByteArray context;
		// Convert the base64 string to a byte array.
		KCodecs::base64Decode(regex.cap(1).utf8(), context);
		if (context.isNull())
		{
			// If we cannot convert the context from
			// a base64 string to bytes, return true.
			parseError = true;
		}
		else
		{
			parameters.insert("Context", context);
		}
	}

	return parseError;
}

void SessionClient::onByeRequest(const SlpRequest& request)
{
	Dialog *dialog = getDialogByCallId(QUuid(request.headers()["Call-ID"].toString()));
	if (dialog == 0l)
	{
		kdDebug() << k_funcinfo << "Got unknown call id on BYE request -- responding with 481" << endl;

		// Generate a No Such Call response
		SlpResponse response = buildResponse(SlpResponse::NoSuchCall, "No Such Call", "application/x-msnmsgr-session-failure-respbody", request);
		// Set the response body.
		const QString responseBody = QString::fromLatin1("SessionID: 0\r\n\r\n");
		response.setBody(responseBody);
		// Set the response context information.
		response.setId(0);
		response.setCorrelationId(request.id());

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
			SlpResponse response = buildResponse(SlpResponse::InternalError, "Internal Error", "null", request);
			// Set the response context information.
			response.setId(0);
			response.setCorrelationId(request.id());

			// Send the 500 Internal Error status response message.
			send(response, 0);
		}
		else
		{
			// Otherwise, we have received a valid BYE request.
			kdDebug() << k_funcinfo << "Got BYE request for dialog, call id = " << dialog->callId().toString().upper() << endl;

			QMap<QString, QVariant> sessionCloseBody;
			if (parseSessionCloseBody(request.body(), sessionCloseBody))
			{
				// TODO Handle parse error in BYE request body.
			}

			// Terminate the dialog.
			dialog->terminate();
		}
	}
}

bool SessionClient::parseDirectConnectionRequestBody(const QString& requestBody, QMap<QString, QVariant> & parameters)
{
	bool parseError = false;
	QRegExp regex("Bridges: ([a-zA-Z0-9 ]*)");
	// Check if the 'Bridges' field is valid.
	if (regex.search(requestBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("Bridges", regex.cap(1));
	}

	regex = QRegExp("NetID: (\\-?\\d+)");
	// Check if the 'NetID' field is valid.
	if (regex.search(requestBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("NetID", regex.cap(1));
	}

	regex = QRegExp("UPnPNat: ([a-z]*)");
	// Check if the 'UPnPNat' field is valid.
	if (regex.search(requestBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("UPnPNat", (QString::fromLatin1("true") == regex.cap(1)));
	}

	regex = QRegExp("Conn-Type: ([a-zA-Z\\-]*)");
	// Check if the 'Conn-Type' field is valid.
	if (regex.search(requestBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("Conn-Type", regex.cap(1));
	}

	regex = QRegExp("SessionID: (\\d+)");
	// Check if the 'SessionID' field is valid.
	if (regex.search(requestBody) != -1)
	{
		parameters.insert("SessionID", regex.cap(1));
	}

	regex = QRegExp("ICF: ([a-z]*)");
	// Check if the 'ICF' field is valid.
	if (regex.search(requestBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("ICF", (QString::fromLatin1("true") == regex.cap(1)));
	}

	return parseError;
}

void SessionClient::onDirectConnectionSetupRequest(const SlpRequest& request)
{
	// Determine if an existing dialog exists.
	Dialog *dialog = getDialogByCallId(QUuid(request.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Established)
	{
		// Check if this transaction for a Direct Connection
		// setup overlaps with one that we recently sent.
		if (isMyDirectConnectionSetupRequestLoser(dialog, request) == false)
		{
			// NOTE If we dropped the peer's direct connection setup request,
			// no response for the request is sent to the peer as this will
			// indicate that we are not honoring their request.
			return;
		}

		kdDebug() << k_funcinfo << "Got Direct Connection setup INVITE request" << endl;

		// Get the request body.
		const QString & requestBody = request.body();

		QMap<QString, QVariant> parameters;
		// Parse the direct connection setup description from the request body
		bool parseError = parseDirectConnectionRequestBody(requestBody, parameters);
		if (parseError)
		{
			// If a parse error is encountered, inform the peer
			// by responding with an internal error.
			kdDebug() << k_funcinfo << "Got invalid direct connection setup description on INVITE request -- responding with 500" << endl;

			// Generate an Internal Error response
			SlpResponse response = buildResponse(SlpResponse::InternalError, "Internal Error", "null", request);
			// Set the response context information.
			response.setId(0);
			response.setCorrelationId(request.id());

			// Send the 500 Internal Error response message.
			send(response, 0);

			// Wait 5 seconds for an acknowledge before termining the dialog.
			// If a timeout occurs, terminate the dialog anyway.
			dialog->terminate(5000);
			return;
		}

		////////////////////////////////////////////
		// Supported bridge types: TRUDPv1 or TCPv1
		////////////////////////////////////////////
		const QValueList<QString> supportedBridgeTypes = QStringList::split(' ', parameters["Bridges"].toString());

		//////////////////////////////////////////////////////////////////////////////////
		// NetID is the big endian version of the sender's internet visible (external)
		// ip address.  For connection types Direct-Connect, Unknown-Connect and Firewall
		// however, NetID is always 0.
		//////////////////////////////////////////////////////////////////////////////////
		const Q_UINT32 netId = parameters["NetID"].toInt();
		const QHostAddress peerIpAddress(ntohl(netId));

		//////////////////////////////////////////////////////////////////
		// Indicates whether the peer is connected to the internet
		// through a router that supports the Universal Plug and Play
		// (UPnp) specification.
		//////////////////////////////////////////////////////////////////
		const bool uPnpNAT = parameters["UPnPNat"].toBool();

		///////////////////////////////////////////////////////////////////////////////////////////
		// Unknown-Connect  (0)	The peer's connection scheme to the internet unknown.
		// Direct-Connect		The peer is directly connected to the internet
		// Unknown-NAT		(3)	The peer is connected to the internet through an NAT is unknown.
		// IP-Restrict-NAT		The peer is connected to the internet through an IP restricted NAT
		// Port-Restrict-NAT(5)	The peer is connected to the internet through a port restricted NAT
		// Symmetric-NAT    (6)	The peer is connected to the internet through a symmetric NAT
		// Firewall				The peer is behind a firewall.
		///////////////////////////////////////////////////////////////////////////////////////////
		const QString connectionType = parameters["Conn-Type"].toString();

		QString sessionId;
		if (parameters.contains("SessionID"))
		{
			// The SessionID field was sent in the request body.
			sessionId = parameters["SessionID"].toString();
		}
		else
		{
			sessionId = QString::number(dialog->session);
		}

		/////////////////////////////////////////////////////
		// Indicates whether the peer is behind an internet
		// connection firewall.
		/////////////////////////////////////////////////////
		const bool behindFirewall = parameters["ICF"].toBool();

		// TODO Handle Hashed-Nonce field
		QUuid nonce;

		// Check whether the peer endpoint is on the same network as us.
		const bool sameNetwork = (peerIpAddress.toString() == d->externalIpAddress);

		SlpResponse response;
		// Check the peer's direct connectivity parameters and ours to determine
		// whether we can establish a direct connection.
		bool allowDirect = supportsDirectConnectivity(connectionType, behindFirewall, uPnpNAT, sameNetwork);
		if (false && allowDirect && supportedBridgeTypes.contains("TCPv1"))
		{
			// If the peer's connectivity scheme is good enough to establish
			// a direct connection, accept the direct connection setup request.
			response = buildResponse(SlpResponse::OK, "OK", "application/x-msnmsgr-transrespbody", request);
			// Build the direct connection response description.
			const QString dccResponseBody = buildDirectConnectionSetupResponseBody(nonce);
			response.setBody(dccResponseBody);
		}
		else
		{
			// Otherwise, decline the direct connection request.
			response = buildResponse(SlpResponse::Decline, "DECLINE", "application/x-msnmsgr-transrespbody", request);
			// Set the response body.
			const QString responseBody = QString("SessionID: %1\r\n\r\n").arg(sessionId);
			response.setBody(responseBody);

			// Start the session since we dropped our direct connection setup
			// request to honor the peer's request and we just declined the
			// peer's direct connection setup request.

			// TODO Update the session state information.
			// TODO Start the session.
		}

		// Set the response context information.
		response.setId(0);
		response.setCorrelationId(request.id());

		// Send the status response message.
		send(response, 0, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Once a dialog has been established, any participant (peer) can initiate a Direct Connection setup request.
// For this reason, it is required to check if an overlapping Direct Connection setup request occurs. This
// is done my comparing the NetID fields in both request bodies.  The NetID which is greater determines who
// wins and therefore who must discard their request.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SessionClient::isMyDirectConnectionSetupRequestLoser(Dialog *dialog, const SlpRequest& request)
{
	bool isMyRequestLoser = true;
	Transaction *transaction = dialog->transactions().last();
	if (transaction->request().method() == QString::fromLatin1("INVITE") &&
		transaction->request().contentType() == QString::fromLatin1("application/x-msnmsgr-transreqbody"))
	{
		kdDebug() << k_funcinfo << "Got overlapping Direct Connection setup request" << endl;

		// Retrieve the peer's Net ID from the request body.
		QRegExp regex("NetID: (\\-?\\d+)");
		regex.search(request.body());
		const Q_INT32 peerNetId = regex.cap(1).toInt();

		// Compare the peer's Net ID to ours.
		isMyRequestLoser = (d->netId < peerNetId);
		if (isMyRequestLoser)
		{
			kdDebug() << k_funcinfo << "Responding to peer's request and dropping mine since peer's id is greater." << endl;
			// Discard our direct connection setup transaction.
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
	// TODO Handle the hashed nonce field.
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
		// If the listening endpoint has been established, write the listening
		// endpoint information to the stream including a nonce to uniquely
		// authenticate incoming direct connections for this endpoint.
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

///////////////////////////////////////////////////////////////////////////////////////////////////////
// A Direct Connection offer request is sent if we responded to a Direct Connection setup request
// and specified that we were not listening on any port because we could not establish a listening
// endpoint.  For this reason, the peer offers to be the listening endpoint and the received request
// contains the peer's listening endpoint information.
///////////////////////////////////////////////////////////////////////////////////////////////////////
void SessionClient::onDirectConnectionOfferRequest(const SlpRequest& request)
{
	// Determine if an established dialog exists.
	Dialog *dialog = getDialogByCallId(QUuid(request.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Established)
	{
		kdDebug() << k_funcinfo << "Got Direct Connection offer INVITE request" << endl;

		// Get the request body.
		const QString & requestBody = request.body();

		// Parse the direct connection description from the request body.
		QMap<QString, QVariant> parameters;
		bool parseError = parseDirectConnectionDescription(requestBody, parameters);
		if (parseError)
		{
			kdDebug() << k_funcinfo << "Got invalid direct connection description on INVITE offer request - responding with 603" << endl;

			// Decline the direct connection offer request.
			SlpResponse response = buildResponse(SlpResponse::Decline, "DECLINE", "application/x-msnmsgr-transrespbody", request);
			// Create the response body.
			const QString responseBody = QString("SessionID: %1\r\n\r\n").arg(dialog->session);
			// Set the response body.
			response.setBody(responseBody);
			// Set the response context information.
			response.setId(0);
			response.setCorrelationId(request.id());

			// Send the 200 OK status response message.
			send(response, 0, 0);

			return;
		}

		kdDebug() << "************* BEGIN Direct Connection setup information *************" << endl;

		// NOTE The bridge type will be the one that we selected
		// associated with the received DCC setup request.
		const QString bridge = parameters["Bridge"].toString();
		kdDebug() << k_funcinfo << "Got bridge, " << bridge << endl;

		// NOTE Indicates whether the peer's transport endpoint is
		// listening for incoming connections.
		const bool isListening = parameters["Listening"].toBool();
		kdDebug() << k_funcinfo << "Got bridge listening, " << isListening << endl;

		const QUuid nonce = QUuid(parameters["Nonce"].toString());
		kdDebug() << k_funcinfo << "Got nonce, " << nonce.toString().upper() << endl;

		const QValueList<QString> externalIpAddresses =
			QStringList::split(' ', parameters["IPv4External-Addrs"].toString());
		kdDebug() << k_funcinfo << "Got external ip address range, " << externalIpAddresses[0] << endl;

		if (d->externalIpAddress == externalIpAddresses[0])
		{
			kdDebug() << k_funcinfo << "Detected peer is on same network" << endl;
		}

		const QString externalPort = parameters["IPv4External-Port"].toString();
		kdDebug() << k_funcinfo << "Got external port, " << externalPort << endl;

		const QValueList<QString> internalIpAddresses =
			QStringList::split(' ', parameters["IPv4Internal-Addrs"].toString());
		kdDebug() << k_funcinfo << "Got internal ip address range, " << internalIpAddresses[0] << endl;

		const QString internalPort = parameters["IPv4Internal-Port"].toString();
		kdDebug() << k_funcinfo << "Got internal port, " << internalPort << endl;

		QString sessionId;
		if (parameters.contains("SessionID"))
		{
			// The SessionID field was sent in the request body.
			sessionId = parameters["SessionID"].toString();
			kdDebug() << k_funcinfo << "Got session id, " << sessionId << endl;
		}
		else
		{
			sessionId = QString::number(dialog->session);
		}

		kdDebug() << "************* END Direct Connection setup information   *************" << endl;

		if (isListening)
		{
			// Create the 200 OK response message
			SlpResponse response = buildResponse(SlpResponse::OK, "OK", "application/x-msnmsgr-transrespbody", request);
			// Set the response body.
			const QString responseBody = QString("SessionID: %1\r\n\r\n").arg(sessionId);
			response.setBody(responseBody);
			// Set the response context information.
			response.setId(0);
			response.setCorrelationId(request.id());

			// Send the 200 OK status response message.
			send(response, 0, 0);

			// Try to create a transport for the session to send/receive its data more efficiently.
			createDirectConnection(bridge, externalIpAddresses, externalPort, nonce, sessionId);
			// TODO Update the session state information.
		}
		else
		{
			kdDebug() << k_funcinfo << "Got Direct Connection offer INVITE request,"
			<< " but peer is not listening -- dropping request." << endl;
		}
	}
}

bool SessionClient::supportsDirectConnectivity(const QString& connectionType, bool behindFirewall, bool uPnpNAT, bool sameNetwork)
{
	Q_UNUSED(sameNetwork);

	bool bSupportsDirectConnectivity = false;
	if (connectionType == QString::fromLatin1("Direct-Connect"))
	{
		bSupportsDirectConnectivity = !behindFirewall;
	}
	else
	if (connectionType == QString::fromLatin1("IP-Restrict-NAT"))
	{
		// IP addresses are different, but external and internal ports are the same.
		bSupportsDirectConnectivity = (!behindFirewall && uPnpNAT);
	}
	else
	if (connectionType == QString::fromLatin1("Symmetric-NAT"))
	{
		// IP addresses are different, and the external and internal ports are different.
		bSupportsDirectConnectivity = (!behindFirewall && uPnpNAT);// && !sameNetwork);
	}
	else
	if (connectionType == QString::fromLatin1("Unknown-NAT"))
	{
		bSupportsDirectConnectivity = (!behindFirewall && uPnpNAT);// && !sameNetwork);
	}


	return bSupportsDirectConnectivity;
}

const QString SessionClient::buildDirectConnectionSetupRequestBody(const Q_UINT32 sessionId)
{
	const QString CrLf = QString::fromLatin1("\r\n");

	QString requestBody;
	QTextStream stream(requestBody, IO_WriteOnly);

	// Write the body content to the stream
	stream << QString::fromLatin1("Bridges: ") << d->supportedBridgeTypes << CrLf;
	stream << QString::fromLatin1("NetID: ") << d->netId << CrLf;
	stream << QString::fromLatin1("Conn-Type: ") << d->connectionType << CrLf;
	stream << QString::fromLatin1("UPnPNat: ") << (d->uPnpNAT ? "true" : "false") << CrLf;
	stream << QString::fromLatin1("ICF: ") << "false" << CrLf;
	stream << QString::fromLatin1("SessionID: ") << sessionId << CrLf;

	return requestBody;
}

bool SessionClient::parseSessionRequestBody(const QString& requestBody, QMap<QString, QVariant> & parameters)
{
	bool parseError = false;
	QRegExp regex("EUF-GUID: \\{([0-9A-F\\-]*)\\}");
	// Check if the euf guid field is valid.
	if (regex.search(requestBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("EUF-GUID", regex.cap(1));
	}

	regex = QRegExp("SessionID: (\\d+)");
	// Check if the session id field is valid.
	if (regex.search(requestBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("SessionID", regex.cap(1));
	}

	regex = QRegExp("AppID: (\\d+)");
	// Check if the app id field is valid.
	if (regex.search(requestBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("AppID", regex.cap(1));
	}

	regex = QRegExp("Context: ([0-9a-zA-Z+/=]*)");
	// Check if the context field is valid.
	if (regex.search(requestBody) == -1)
	{
		parseError = true;
	}
	else
	{
		QByteArray context;
		// Convert the base64 string to a byte array.
		KCodecs::base64Decode(regex.cap(1).utf8(), context);
		if (context.isNull())
		{
			// If we cannot convert the context from
			// a base64 string to bytes, return true.
			parseError = true;
		}
		else
		{
			parameters.insert("Context", context);
		}
	}

	return parseError;
}

void SessionClient::onInitialInviteRequest(const SlpRequest& request)
{
	Dialog *dialog = getDialogByCallId(QUuid(request.headers()["Call-ID"].toString()));
	if (dialog != 0l)
	{
		kdDebug() << "Got duplicate initial INVITE request for dialog, call id = "
		<< dialog->callId().toString().upper() << " -- ignoring it" << endl;
		return;
	}

	// Create the dialog from the INVITE request.
	dialog = new Dialog(new Transaction(request, false), this);
	// Add the pending dialog to the call map.
	addDialogToCallMap(dialog);

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

		// Generate an Internal Error response
		SlpResponse response = buildResponse(SlpResponse::InternalError, "Internal Error", "null", request);
		// Set the response context information.
		response.setId(0);
		response.setCorrelationId(request.id());

		// Send the 500 Internal Error response message.
		send(response, 0);

		// Wait 15 seconds for an acknowledge before termining the dialog.
		// If a timeout occurs, terminate the dialog anyway.
		dialog->terminate(15000);
		return;
	}

	// Retrieve the session decription parameters.
	const QUuid uuid = QUuid(sessionDescription["EUF-GUID"].toString());
	const Q_UINT32 sessionId = sessionDescription["SessionID"].toInt();
	const Q_UINT32 appId = sessionDescription["AppID"].toInt();
	const QByteArray context = sessionDescription["Context"].toByteArray();

	// Try to select the appropriate handler for the session
	// invitation based on uuid.
	Session *session = createSession(sessionId, uuid);
	if (session)
	{
		// If there is a registered application that can handle
		// the received session description (offer), pass the offer
		// to the application.
		dialog->session = sessionId;
		// Add the session to the list.
		d->sessions.insert(sessionId, session);

		// Connect the signal/slot.
		QObject::connect(session , SIGNAL(accepted()), this,
		SLOT(onSessionAccept()));
		QObject::connect(session , SIGNAL(declined()), this,
		SLOT(onSessionDecline()));

		// Handle the incoming invitation notification.
		session->handleInvite(appId, context);
	}
	else
	{
		// Otherwise, decline the session.
		kdDebug() << "Got initial INVITE request for unsupported application" << endl;

		declineSession(sessionId);
	}
}

Session* SessionClient::createSession(const Q_UINT32 sessionId, const QUuid& uuid)
{
	Session *session = 0l;
	SessionNotifier *notifier = 0l;

	if (uuid == QUuid(MsnObjectSession::staticMetaObject()->classInfo("EUF-GUID")))
	{
		session = new MsnObjectSession(sessionId, Session::Outgoing, this);
		notifier = new SessionNotifier(session->id(), SessionNotifier::Object, this);
		// Connect the signal/slot
		QObject::connect(notifier, SIGNAL(messageReceived(const QByteArray&, const Q_INT32, const Q_INT32)), session,
		SLOT(onReceive(const QByteArray&, const Q_INT32, const Q_INT32)));
		QObject::connect(notifier, SIGNAL(messageAcknowledged(const Q_INT32)), session,
		SLOT(onSend(const Q_INT32)));
	}
	else
	if (uuid == QUuid(FileTransferSession::staticMetaObject()->classInfo("EUF-GUID")))
	{
		session = new FileTransferSession(sessionId, Session::Incoming, d->peer, this);
		notifier = new SessionNotifier(session->id(), SessionNotifier::FileTransfer, this);
		// Connect the signal/slot
		QObject::connect(notifier, SIGNAL(dataReceived(const QByteArray&, bool)), session,
 		SLOT(onDataReceived(const QByteArray&, bool)));
	}

	if (session != 0 && notifier != 0l)
	{
		// Register the notifier for the session
		d->transport->registerPort(notifier->session(), notifier);
	}

	return session;
}

//END

//BEGIN Response Handling Functions

//////////////////////////////////////////////////////////////////////
// Handles session layer protocol responses received.
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
	const SlpResponse::StatusCode statusCode = (SlpResponse::StatusCode)response.statusCode();
	if (statusCode == SlpResponse::OK)
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
	if (statusCode == SlpResponse::NotFound)
	{
		// NOTE MSNSLP/1.0 404 Not Found
		onRecipientUriNotFound(response);
	}
	else
	if (statusCode == SlpResponse::NoSuchCall)
	{
		// NOTE MSNSLP/1.0 481 No Such Call
		onNoSuchCall(response);
	}
	else
	if (statusCode == SlpResponse::InternalError)
	{
		// NOTE MSNSLP/1.0 500 Internal Error
		onInternalError(response);
	}
	else
	if (statusCode == SlpResponse::Decline)
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

bool SessionClient::parseDirectConnectionDescription(const QString& messageBody, QMap<QString, QVariant> & parameters)
{
	bool parseError = false;

	QRegExp regex("Bridge: ([a-zA-Z0-9 ]*)");
	// Check if the bridge field is valid.
	if (regex.search(messageBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("Bridge", regex.cap(1));
	}

	regex = QRegExp("Listening: ([a-z]*)");
	// Check if the listening field is valid.
	if (regex.search(messageBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("Listening", (QString::fromLatin1("true") == regex.cap(1)));
	}

	regex = QRegExp("Nonce: \\{([0-9A-F\\-]*)\\}");
	// Check if the nonce field is valid.
	if (regex.search(messageBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("Nonce", regex.cap(1));
	}

	regex = QRegExp("IP[Vv]4External-Addrs: ([^\r\n]*)");
	// Check if the external ipv4 addresses field is valid.
	if (regex.search(messageBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("IPv4External-Addrs", regex.cap(1));
	}

	regex = QRegExp("IP[Vv]4External-Port: (\\d+)");
	// Check if the external ipv4 port field is valid.
	if (regex.search(messageBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("IPv4External-Port", regex.cap(1));
	}

	regex = QRegExp("IP[Vv]4Internal-Addrs: ([^\r\n]*)");
	// Check if the internal ipv4 addresses field is valid.
	if (regex.search(messageBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("IPv4Internal-Addrs", regex.cap(1));
	}

	regex = QRegExp("IP[Vv]4Internal-Port: (\\d+)");
	// Check if the internal ipv4 port field is valid.
	if (regex.search(messageBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("IPv4Internal-Port", regex.cap(1));
	}

	regex = QRegExp("SessionID: (\\d+)");
	// Check if the session id field is valid.
	if (regex.search(messageBody) == -1)
	{
		parseError = true;
	}
	else
	{
		parameters.insert("SessionID", regex.cap(1));
	}

	return parseError;
}

void SessionClient::onDirectConnectionRequestAccepted(const SlpResponse& response)
{
	// Determine if an existing dialog exists.
	Dialog *dialog = getDialogByCallId(QUuid(response.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Established)
	{
		// Get the response body.
		const QString responseBody = response.body();

		/////////////////////////////////////////////////////////////////
		// Retrieve the last transaction associated with this response
		// to determine if the request was a Direct Connection setup or
		// a Direct Connection offer request.
		/////////////////////////////////////////////////////////////////

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		// Get the transaction associated with this response.
		Transaction *transaction = d->transactions[branch];
		// Get the request associated with this response from the transaction.
		const SlpRequest & request = transaction->request();

		if (request.contentType() == QString::fromLatin1("application/x-msnmsgr-transreqbody"))
		{
			// If the response is to a Direct Connection setup request,
			// retrieve the data from the response body to setup the
			// direct connection.
			kdDebug() << k_funcinfo << "Got Direct Connection setup request accepted" << endl;

			// Parse the direct connection description from the response body.
			QMap<QString, QVariant> parameters;
			bool parseError = parseDirectConnectionDescription(responseBody, parameters);
			if (parseError)
			{
				kdDebug() << k_funcinfo << "Got invalid direct connection description" << endl;
				return;
			}

			kdDebug() << "************* BEGIN Direct Connection setup information *************" << endl;

			// NOTE The bridge type will be the one that we offered
			// associated with the received Direct Connection setup request.
			const QString bridge = parameters["Bridge"].toString();
			kdDebug() << k_funcinfo << "Got bridge, " << bridge << endl;

			// NOTE Indicates whether the peer's transport endpoint is
			// listening for incoming connections.
			const bool isListening = parameters["Listening"].toBool();
			kdDebug() << k_funcinfo << "Got bridge listening, " << isListening << endl;

			const QUuid nonce = QUuid(parameters["Nonce"].toString());
			kdDebug() << k_funcinfo << "Got nonce, " << nonce.toString().upper() << endl;

			const QValueList<QString> externalIpAddresses =
				QStringList::split(' ', parameters["IPv4External-Addrs"].toString());
			kdDebug() << k_funcinfo << "Got external ip address range, " << externalIpAddresses[0] << endl;

			if (d->externalIpAddress == externalIpAddresses[0])
			{
				kdDebug() << k_funcinfo << "Detected peer is on same network" << endl;
			}

			const QString externalPort = parameters["IPv4External-Port"].toString();
			kdDebug() << k_funcinfo << "Got external port, " << externalPort << endl;

			const QValueList<QString> internalIpAddresses =
				QStringList::split(' ', parameters["IPv4Internal-Addrs"].toString());
			kdDebug() << k_funcinfo << "Got internal ip address range, " << internalIpAddresses[0] << endl;

			const QString internalPort = parameters["IPv4Internal-Port"].toString();
			kdDebug() << k_funcinfo << "Got internal port, " << internalPort << endl;

			QString sessionId;
			if (parameters.contains("SessionID"))
			{
				// The SessionID field was sent in the response body.
				sessionId = parameters["SessionID"].toString();
				kdDebug() << k_funcinfo << "Got session id, " << sessionId << endl;
			}
			else
			{
				sessionId = QString::number(dialog->session);
			}

			kdDebug() << "************* END Direct Connection setup information   *************" << endl;

			if (isListening)
			{
				// Try to create a transport for the session to send/receive its data more efficiently.
				createDirectConnection(bridge, externalIpAddresses, externalPort, nonce, sessionId);

				// TODO Update the session state information.
			}
			else
			{
				// The peer has indicated that they are unable to create
				// a listening endpoint, so we will try to be the listening
				// endpoint.
				kdDebug() << k_funcinfo << "Peer not listening, offering Direct Connection endpoint" << endl;

				// Build a direct connection offer request with our listening
				// endpoint information.
				SlpRequest request = buildRequest("INVITE", QString::fromLatin1("application/x-msnmsgr-transrespbody"), dialog);
				const QUuid cnonce = QUuid::createUuid();
				const QString responseBody = buildDirectConnectionSetupResponseBody(cnonce);
				// Set the request body.
				request.setBody(responseBody);

				// Create a new transaction for the request.
				Transaction *t = new Transaction(request, true);
				dialog->transactions().append(t);

				// Begin the transaction.
				beginTransaction(t);

				// Send the transaction request.
				send(t->request(), 0, 0);
			}
		}
		else
		if (request.contentType() == QString::fromLatin1("application/x-msnmsgr-transrespbody"))
		{
			// If the response is to a Direct Connection offer request,
			// update the session state information.

			kdDebug() << k_funcinfo << "Got Direct Connection offer request accepted" << endl;

			// TODO Update the session state information.
		}

		// End the transaction.
		endTransaction(transaction);
	}
}

void SessionClient::onDirectConnectionRequestDeclined(const SlpResponse& response)
{
	// Determine if an established dialog exists.
	Dialog *dialog = getDialogByCallId(QUuid(response.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Established)
	{
		kdDebug() << k_funcinfo << "Got Direct Connection request declined" << endl;

		// Get the response body.
		const QString responseBody = response.body();

		QRegExp regex("SessionID: (\\d+)");
		regex.search(responseBody);
		const QString sessionId = regex.cap(1);

		// TODO Update the session state information.
// 		fireDirectConnectionSetupRequestFailed(sessionId);

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
	if (dialog != 0l && dialog->state() != Dialog::Terminated)
	{
		// If the dialog exists, terminate the dialog and the transaction
		// associated with the received response.
		kdDebug() << k_funcinfo << "Got Internal Error for dialog call id = "
		<< dialog->callId().toString().upper() << " -- terminating" << endl;

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		// End the transaction since an error has occured.
		Transaction *transaction = d->transactions[branch];
		endTransaction(transaction);

		// Terminate the dialog and its associated session.
		dialog->terminate();
	}
}

void SessionClient::onNoSuchCall(const SlpResponse& response)
{
	kdDebug() << k_funcinfo << "called" << endl;
	// Determine if an established dialog exists.  If one exists, terminate the dialog.
	Dialog *dialog = getDialogByCallId(QUuid(response.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Established)
	{
		kdDebug() << k_funcinfo << "Got No Such Call for dialog call id = "
		<< dialog->callId().toString().upper() << " -- terminating" << endl;

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		// End the transaction since an error has occured.
		Transaction *transaction = d->transactions[branch];
		endTransaction(transaction);

		// Terminate the dialog and its associated session
		dialog->terminate();
	}
}

void SessionClient::onRecipientUriNotFound(const SlpResponse& response)
{
	// Try to retrieve the dialog associated with this response.
	Dialog *dialog = getDialogByCallId(QUuid(response.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() != Dialog::Terminated)
	{
		// If the dialog exists, terminate the dialog and the transaction
		// associated with the received response.
		kdDebug() << k_funcinfo << "Got recipient Uri Not Found for dialog call id = "
		<< dialog->callId().toString().upper() << " -- terminating" << endl;

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		// End the transaction since an error has occured.
		Transaction *transaction = d->transactions[branch];
		endTransaction(transaction);

		// Terminate the dialog and its associated session
		dialog->terminate();
	};
}

void SessionClient::onSessionRequestAccepted(const SlpResponse& response)
{
	// Determine if a pending dialog exists.
	Dialog *dialog = getDialogByCallId(QUuid(response.headers()["Call-ID"].toString()));
	if (dialog != 0l && dialog->state() == Dialog::Pending)
	{
		kdDebug() << k_funcinfo << "Session ACCEPTED, id = " << dialog->session << endl;
		// Set the dialog state to established.
		dialog->establish();

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		Transaction *transaction = d->transactions[branch];
		// End the transaction.
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
		kdDebug() << k_funcinfo << "Session DECLINED, id = " << dialog->session << endl;

		// Get the transaction branch identifier.
		const QUuid branch = getTransactionBranchFrom(response);
		Transaction *transaction = d->transactions[branch];
		// Check whether the transaction associated with the response
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

void SessionClient::onSessionAccept()
{
	const Session *session = dynamic_cast<const Session*>(sender());
	if (session != 0l)
	{
		// Send a session acceptance notification to the peer endpoint.
		acceptSession(session->id());
	}
}

void SessionClient::onSessionCancel()
{
	const Session *session = dynamic_cast<const Session*>(sender());
	if (session != 0l)
	{
		// Send a session cancellation notification to the peer endpoint.
		closeSessionInternal(session->id());
	}
}

void SessionClient::onSessionDecline()
{
	const Session *session = dynamic_cast<const Session*>(sender());
	if (session != 0l)
	{
		// Send a session decline notification to the peer endpoint.
		declineSession(session->id());
	}
}

void SessionClient::onSessionEnd()
{
	const Session *session = dynamic_cast<const Session*>(sender());
	if (session != 0l)
	{
		// Send a session end notification to the peer endpoint.
		closeSessionInternal(session->id());
	}
}

void SessionClient::onSessionSendFile(QFile *file)
{
	const Session *session = dynamic_cast<const Session*>(sender());
	if (session != 0l)
	{
		d->transport->sendFile(file, session->id());
	}
}

void SessionClient::onSessionSendData(const QByteArray& bytes)
{
	const Session *session = dynamic_cast<const Session*>(sender());
	if (session != 0l)
	{
		Dialog *dialog = getDialogBySessionId(session->id());
		if (dialog != 0l)
		{
			d->transport->sendBytes(bytes, session->id(), (rand() % 0xCF6C));
		}
	}
}

void SessionClient::onSessionSendMessage(const QByteArray& bytes)
{
	const Session *session = dynamic_cast<const Session*>(sender());
	if (session != 0l)
	{
		Dialog *dialog = getDialogBySessionId(session->id());
		if (dialog != 0l)
		{
			const Q_UINT32 id = d->transport->send(bytes, session->id(), (rand() % 0xCF6C));
			dialog->setTransactionId(id);
		}
	}
}

//END

void SessionClient::send(SlpMessage& message, const Q_UINT32 destination, const Q_UINT32 priority)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	Q_UNUSED(priority);

	QByteArray bytes;
	QTextStream stream(bytes, IO_WriteOnly);

	const QString CrLf = QString::fromLatin1("\r\n");
	// Write the start line to the stream
	stream << message.startLine() << CrLf;
	// Write the message headers to the stream.
	stream << "To: " << message.headers()["To"].toString() << CrLf;
	stream << "From: " << message.headers()["From"].toString() << CrLf;
	stream << "Via: " << message.headers()["Via"].toString() << CrLf;
	stream << "CSeq: " << message.headers()["CSeq"].toString() << CrLf;
	stream << "Call-ID: " << message.headers()["Call-ID"].toString() << CrLf;
	stream << "Max-Forwards: " << message.headers()["Max-Forwards"].toString() << CrLf;
	stream << "Content-Type: " << message.headers()["Content-Type"].toString() << CrLf;

	// Get the message content length.
	Q_INT32 contentLength = message.body().length();
	contentLength = (contentLength != 0) ? contentLength + 1 : contentLength;
	// Write the message content length to the stream.
	stream << QString::fromLatin1("Content-Length: ") << contentLength << CrLf
	<< CrLf;

	if (contentLength > 0)
	{
		// If the message has a body, write the body content to the stream.
		stream << message.body() << '\0';
	}

	kdDebug() << k_funcinfo << QString(bytes) << endl;

	// Send the message data bytes via the transport layer.
	const Q_UINT32 id = d->transport->send(bytes, destination, message.correlationId());
	message.setId(id);

	Dialog *dialog = getDialogByCallId(QUuid(message.headers()["Call-ID"].toString()));
	if (dialog != 0l)
	{
		dialog->setTransactionId(id);
	}

	kdDebug() << k_funcinfo << "leave" << endl;
}

//BEGIN Transport Handling Functions

void SessionClient::createDirectConnection(const QString& type, const QValueList<QString> & ipAddresses, const QString& port, const QUuid& nonce, const QString& sessionId)
{
	kdDebug() << k_funcinfo << "enter" << endl;
	kdDebug() << k_funcinfo << "Session " << sessionId << endl;

	d->transport->createDirectBridge(type, ipAddresses, port.toInt(), nonce);
	kdDebug() << k_funcinfo << "leave" << endl;
}

//END

//BEGIN Image Message Handling Functions

void SessionClient::onGifImageReceived(const Message& message)
{
	// Get the message body.
	const QString & messageBody = message.body();
	// Try to retrieve the base64 body parameter.
	QRegExp regex("base64:([0-9a-zA-Z+/=]*)");
	if (regex.search(messageBody) != -1)
	{
		QByteArray bytes;
		// Convert from base64 string to a byte array
		KCodecs::base64Decode(regex.cap(1).utf8(), bytes);

		// Create a temporary file to store the data in.
		KTempFile *temporaryFile = new KTempFile(locateLocal("tmp", "image-gif-"), ".gif");
		QDataStream *stream = temporaryFile->dataStream();
		if (stream)
		{
			// Write the data to the temporary file.
			stream->writeRawBytes(bytes.data(), bytes.size());
		}
		// Close the file.
		temporaryFile->close();

		emit imageReceived(temporaryFile);
		temporaryFile = 0l;
	}
}

//////////////////////////////////////////////////////////////////////
// Handles session layer protocol image data messages received
// Supported types: image/gif.
//////////////////////////////////////////////////////////////////////
void SessionClient::onImageReceived(const QByteArray& content, const Q_INT32 id, const Q_INT32 correlationId)
{
	Q_UNUSED(id);
	Q_UNUSED(correlationId);

	// Get the UTF16 (unicode) encoding string.
	const QString msg = QString::fromUcs2((unsigned short*)((void*)content.data()));

	Message message;
	QMap<QString, QVariant> headers;
	// Parse the message headers from the raw data.
	Message::parseHeaders(msg.section("\r\n\r\n", 0, 0), headers);
	// Copy the parsed headers to the message's header collection.
	message.copyHeadersFrom(headers);
	// Set the message body content.
	const QString messageBody = msg.section("\r\n\r\n", 1, 1, QString::SectionIncludeTrailingSep);
	message.setBody(messageBody);

	if (message.contentType() == QString::fromLatin1("image/gif"))
	{
		// Handle the gif image received.
		onGifImageReceived(message);
	}
}

void SessionClient::sendImage(const QString& path)
{
	Q_UNUSED(path);

	// TODO Support the sending of gif images.  Currently,
	// Kopete has no means of capturing handwriting/ink.
}

//END

}

#include "sessionclient.moc"
