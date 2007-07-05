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
#include <qvaluelist.h>
#include "slprequest.h"
#include "slpresponse.h"

class KTempFile;
namespace Kopete { class Contact; }
class QFile;

namespace PeerToPeer
{

class Dialog;
class Session;
class Transaction;
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
		SessionClient(const QMap<QString, QVariant> & properties, Kopete::Contact* me, Kopete::Contact *peer, Transport* transport, QObject *parent);
		~SessionClient();

		/** @brief Returns a value indicating whether the client is active. */
		bool isActive() const;
		/** @brief Creates a session to request the specified msn object. */
		void requestObject(const QString& object);
		/** @brief Sends a gif image message to the peer endpoint. */
		void sendImage(const QString& path);
		/** @brief Creates a session to send the specified file. */
		void sendFile(const QString& path);

	signals:
		/** @brief Occurs when an image is received. */
		void imageReceived(KTempFile* temporaryFile);
		/** @brief Occurs when a msn object is received. */
		void objectReceived(const QString& object, KTempFile *temporaryFile);

	private slots:
		/** @brief Called when a dialog is established. */
		void onDialogEstablish();
		/** @brief Called when a dialog terminates. */
		void onDialogTerminate();
		/** @brief Called when a gif image is received. */
		void onGifImageReceived(const Message& message);
		/** @brief Called when an image is received. */
		void onImageReceived(const QByteArray& content, const Q_INT32 id, const Q_INT32 correlationId);
		/** @brief Called when a message acknowledge is received for a sent message. */
		void onSend(const Q_INT32 id);
		/** @brief Called when a message is received. */
		void onReceived(const QByteArray& content, const Q_INT32 id, const Q_INT32 correlationId);
		/** @brief Called when a local dialog transaction times out.*/
		void onTransactionTimeout();

	private slots:
		/** @brief Called when a session has been accepted by the user. */
		void onSessionAccept();
		/** @brief Called when a session has been declined by the user. */
		void onSessionDecline();
		/** @brief Called when a session has ended. */
		void onSessionEnd();
		/** @brief Called when a session wants to send a message. */
		void onSessionSendMessage(const QByteArray& bytes);
		/** @brief Called when a session wants to send raw data. */
		void onSessionSendData(const QByteArray& bytes);
		/** @brief Called when a session wants to send a file. */
		void onSessionSendFile(QFile *file);

		/** @brief Called when the transport's default bridge initially connects. */
		void onTransportInitialConnect();

	private:
		/** @brief Accepts a session invitation. */
		void acceptSession(const Q_UINT32 sessionId);
		/** @brief Adds the specified dialog to the call map. */
		void addDialogToCallMap(Dialog *dialog);
		/** @brief Begins a dialog transaction. */
		void beginTransaction(Transaction *transaction);
		/** @brief Builds the parameters of a direct connection setup request message body. */
		const QString buildDirectConnectionSetupRequestBody(const QUuid& nonce, const Q_UINT32 sessionId);
		/** @brief Builds the parameters of a direct connection setup response message body. */
		const QString buildDirectConnectionSetupOrOfferResponseBody(const QString& sessionId, const QUuid& nonce, bool supportsHashedNonce, bool isOffer=false);
		/** @brief Builds a request using the specified method, content type and dialog. */
		SlpRequest buildRequest(const QString& method, const QString& contentType, Dialog *dialog);
		/** @brief Builds a response using the specified status code, status description, content type and request. */
		SlpResponse buildResponse(const Q_INT32 statusCode, const QString& statusDescription, const QString& contentType, const SlpRequest& request);
		/** @brief Builds the parameters of a session setup request message body. */
		const QString buildSessionDescriptionBody(const QUuid& uuid, const Q_UINT32 sessionId, const Q_UINT32 appId, const QString& context);
		/** @brief Creates a remotely initiated session. */
		Session* createSession(const Q_UINT32 sessionId, const QUuid& uuid);
		/** @brief Creates a locally initiated session. */
		void createSessionInternal(const QUuid& uuid, const Q_UINT32 sessionId, const Q_UINT32 appId, const QString& context);
		/** @brief Tries to create a direct transport for the supplied session using the specified information. */
		void createDirectConnection(const QMap<QString, QVariant> & transportInfo);
		Q_INT16 createDirectConnectionListener(const QUuid& nonce, const QUuid& peerNonce, const bool supportsHashedNonce);
		/** @brief Declines a session invitation. */
		void declineSession(const Q_UINT32 sessionId);
		/** @brief Terminates a session. */
		void closeSession(const Q_UINT32 sessionId);
		/** @brief Ends a dialog transaction. */
		void endTransaction(Transaction *transaction);
		/** @brief Gets a dialog based on the call id.*/
		Dialog* getDialogByCallId(const QUuid& callId);
		/** @brief Gets a dialog based on the session id.*/
		Dialog* getDialogBySessionId(const Q_UINT32 sessionId);
		/** @brief Gets a dialog based on the transaction sequence number.*/
		Dialog* getDialogByTransactionId(const Q_UINT32 transactionId);
		/** @brief Indicates whether an overlapping direct connection setup request received or a
		 *		   direct connection setup request recently sent will be dropped.
		 */
		bool isMyDirectConnectionSetupRequestLoser(Dialog *dialog, const SlpRequest& request);
		/** @brief Initializes the session client. */
		void initialize();

		bool parseSessionCloseBody(const QString& requestBody, QMap<QString, QVariant> & parameters);
		bool parseSessionRequestBody(const QString& requestBody, QMap<QString, QVariant> & parameters);
		bool parseDirectConnectionRequestBody(const QString& requestBody, QMap<QString, QVariant> & transportInfo);
		bool parseDirectConnectionResponseBody(const QString& responseBody, QMap<QString, QVariant> & transportInfo);

		/** @brief Called when the remote endpoint wants to terminates the dialog. */
		void onByeRequest(const SlpRequest& request);
		/** @brief Called when the remote endpoint wants to establish a dialog. */
		void onInitialInviteRequest(const SlpRequest& request);
		/** @brief Called when the peer endpoint wants to setup a direct connection. */
		void onDirectConnectionSetupRequest(const SlpRequest& request);
		/** @brief Called when the peer endpoint offers to be the listening endpoint
		 *		   of a direct connection as we have indicated that we cannot.
		 */
		void onDirectConnectionOfferRequest(const SlpRequest& request);
		/** @brief Called when a request message is received. */
		void onRequestMessage(const SlpRequest& request);
		/** @brief Called when a response message is received. */
		void onResponseMessage(const SlpResponse& response);
		/** @brief Called to indicate that an internal error was found in a sent message. */
		void onInternalError(const SlpResponse& response);
		/** @brief Called when the call identifier of a sent message does not match that of any dialog at the peer endpoint. */
		void onNoSuchCall(const SlpResponse& response);
		/** @brief Called when the peer accepts a session setup request. */
		void onSessionRequestAccepted(const SlpResponse& response);
		/** @brief Called when the peer declines a session setup request. */
		void onSessionRequestDeclined(const SlpResponse& response);
		/** @brief Called when the peer accepts a direct connection setup request. */
		void onDirectConnectionRequestAccepted(const SlpResponse& response);
		/** @brief Called when the peer declines a direct connection setup request. */
		void onDirectConnectionRequestDeclined(const SlpResponse& response);
		/** @brief Called when the To uri in a sent message does not match that of the peer endpoint. */
		void onRecipientUriNotFound(const SlpResponse& response);
		/** @brief Gets the transaction branch identifier from the specified message. */
		QUuid getTransactionBranchFrom(const SlpMessage& message);
		/** @brief Removes a dialog with the specified identifier from the call map. */
		void removeDialogFromCallMap(const QUuid& callId);
		/** @brief Sends a direct connection setup request to the peer endpoint. */
		void requestDirectConnection(const Q_UINT32 sessionId);
		/** @brief Sends the supplied message to the specified destination. */
		void send(SlpMessage& message, const Q_UINT32 destination, const Q_UINT32 priority=1);
		void sendErrorResponse(const Q_INT32 statusCode, const QString& statusDescription, const QString& contentType, const SlpRequest& request, const QString& responseBody=QByteArray());
		/** @brief Indicates whether the parameters sent by the peer in a direct connection
		 *		   setup request would support the possible scenarios to establish a direct
         *         connection.
		 */
		bool supportsDirectConnectivity(const QString& connectionType, bool behindFirewall, bool upnpNatPresent);
		/** @brief Gets the next ,unique session id. */
		Q_UINT32 nextSessionId() const;

	private:
		class SessionClientPrivate;
		SessionClientPrivate *d;

}; // SessionClient
}

#endif

