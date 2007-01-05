/*
    sessionclient.h - Peer to Peer Session Client class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__SESSIONCLIENT_H
#define CLASS_P2P__SESSIONCLIENT_H

#include <qobject.h>
#include <qmap.h>
#include <quuid.h>
#include "slprequest.h"
#include "slpresponse.h"
#include "transaction.h"

class KTempFile;

namespace PeerToPeer
{

class Dialog;
class Session;
class Transport;

/**
 * @brief Represents a client implementation that handles session management.
 *
 * This includes creating and terminating sessions, modifying session parameters and invoking services.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class SessionClient : public QObject
{
	Q_OBJECT

	public :
		/** @brief Creates a new instance of the SessionClient class. */
		SessionClient(const QMap<QString, QVariant> & properties, Transport* transport, QObject *parent);
		~SessionClient();

		void requestAvatar(const QString& object);
		const Q_INT32 activeDialogs() const;

	signals:
		/** @brief Occurs when an image is received. */
		void imageReceived(KTempFile* temporaryFile);

	private slots:
		/** @brief Called when a dialog is established. */
		void onDialogEstablish();
		/** @brief Called when a dialog terminates. */
		void onDialogTerminate();
		/** @brief Called when an image is received. */
		void onImageReceived(const QByteArray& content, const Q_INT32 identifier, const Q_INT32 relatesTo);
		/** @brief Called when a message acknowledgement is received. */
		void onSend(const Q_INT32 identifier);
		/** @brief Called when a message is received. */
		void onReceived(const QByteArray& content, const Q_INT32 identifier, const Q_INT32 relatesTo);
		/** @brief Called when a remote dialog transaction times out. */
		void onTransactionTimedout(const Q_INT32 identifier, const Q_INT32 relatesTo);
		/** @brief Called when a local dialog transaction times out.*/
		void onTransactionTimeout();

	private:
		/** @brief Accepts a session invitation. */
		void acceptSession(Dialog *dialog, const Q_INT32 sessionId);
		void addDialogToCallMap(Dialog *dialog);
		/** @brief Begins a dialog transaction. */
		void beginTransaction(Transaction *transaction);
		const QString buildDirectConnectionSetupRequestBody();
		const QString buildDirectConnectionSetupResponseBody(const QUuid& nonce);
		SlpRequest buildRequest(const QString& method, const QString& contentType, Dialog *dialog);
		SlpResponse buildResponse(const Q_INT32 statusCode, const QString& statusDescription, const QString& contentType, const SlpRequest& request);
		const QString buildSessionDescriptionBody(const QUuid& uuid, const Q_UINT32 sessionId, const Q_UINT32 appId, const QString& context);
		/** @brief Gets a value indicating whether direct connectivity is supported. */
		void createSession(const QUuid& uuid, const Q_UINT32 sessionId, const Q_UINT32 appId, const QString& context);
		/** @brief Accepts a session invitation. */
		void declineSession(Dialog *dialog, const Q_INT32 sessionId);
		/** @brief Terminates a session. */
		void endSession(Dialog *dialog, const Q_INT32 sessionId);
		/** @brief Ends a dialog transaction. */
		void endTransaction(Transaction *transaction);
		/** @brief Gets an established dialog based on the call id.*/
		Dialog* getDialogByCallId(const QUuid& identifier);
		Dialog* getDialogBySessionId(const Q_UINT32 sessionId);
		Dialog* getDialogByTransactionId(const Q_UINT32 transactionId);
		bool isMyDirectConnectionSetupRequestLoser(Dialog *dialog, const SlpRequest& request);
		/** @brief Initializes the session client. */
		void initialize();
		/** @brief Returns the path to an msn object. */
		QString locate(const QString& type, const QString& key);
		bool parseSessionCloseBody(const QString& requestBody, QMap<QString, QVariant> & collection);
		bool parseSessionRequestBody(const QString& requestBody, QMap<QString, QVariant> & collection);
		bool parseDirectConnectionRequestBody(const QString& requestBody, QMap<QString, QVariant> & collection);
		bool parseDirectConnectionResponseBody(const QString& responseBody, QMap<QString, QVariant> & collection);

		/** @brief Called when the remote endpoint wants to terminates the dialog. */
		void onByeRequest(const SlpRequest& request);
		/** @brief Called when the remote endpoint wants to establish a dialog. */
		void onInitialInviteRequest(const SlpRequest& request);
		/** @brief Called when the remote endpoint wants to modify the session parameter of the dialog. */
		void onDirectConnectionSetupRequest(const SlpRequest& request);
		void onDirectConnectionOfferRequest(const SlpRequest& request);
		/** @brief Called when a request message is received. */
		void onRequestMessage(const SlpRequest& request);
		/** @brief Called when a response message is received. */
		void onResponseMessage(const SlpResponse& response);
		void onInternalError(const SlpResponse& response);
		void onNoSuchCall(const SlpResponse& response);
		void onSessionRequestAccepted(const SlpResponse& response);
		void onSessionRequestDeclined(const SlpResponse& response);
		void onDirectConnectionRequestAccepted(const SlpResponse& response);
		void onDirectConnectionRequestDeclined(const SlpResponse& response);
		void onRecipientUriNotFound(const SlpResponse& response);
		QUuid getTransactionBranchFrom(const SlpMessage& message);
		void parseHeaders(const QString& input, QMap<QString, QVariant> & headers);
		void removeDialogFromCallMap(const QUuid& identifier);
		/** @brief Sends an acknowledgement notification to the specified destination. */
		void sendAcknowledge(const Q_INT32 destination, const Q_INT32 identifier, const Q_INT32 relatesTo, const Q_UINT32 priority=1);
		void sendImage(const QString& path);
		/** @brief Sends the supplied message to the specified destination. */
		void send(const SlpMessage& message, const Q_UINT32 destination, const Q_UINT32 priority=1);
		bool supportsDirectConnectivity(const QString& connectionType, bool behindFirewall, bool uPnpSupported, bool sameNetwork);

	private:
		class SessionClientPrivate;
		SessionClientPrivate *d;

}; // SessionClient
}

#endif

