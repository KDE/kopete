/*
   tweenerhandler.h - Negociation with Passport to get the login ticket.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONTWEENERHANDLER_H
#define PAPILLONTWEENERHANDLER_H

#include <Papillon/Macros>
#include <QtCore/QObject>

class QHttpRequestHeader;
namespace Papillon
{

class SecureStream;
/**
 * @class TweenerHandler tweenerhandler.h <Papillon/Http/TweenerHandler>
 * @brief Negociation of the Tweener ticket with Passport login server.
 *
 * Using TweenerHandler:
 * TweenerHandler require a SecureStream instance for TLS/SSL connection.
 * Use setLoginInformation() to set required login information and start() to begin negotiation of the tweener.
 *
 * @code
 * TweenerHandler *twn = new TweenerHandler(secureStream);
 * twn->setLoginInformation(tweener, QLatin1String("test@passport.com"), QLatinString("password"));
 * connect(twn, SIGNAL(result(TweenerHandler *)), this, SLOT(tweenerResult(TweenerHandler*)));
 * twn->start();
 * @endcode
 *
 * If success() return false, assume that the password was bad.
 *
 * @author Michaël Larouche <larouche@kde.org>
*/
class PAPILLON_EXPORT TweenerHandler : public QObject
{
	Q_OBJECT
public:
	/**
	 * TweenerState is using to switch the current state of the process.
	 */
	enum TweenerState 
	{
		/**
		 * We are getting the name of the Passport login server.
		 */
		TwnGetServer,
		/**
		 * Sending authentication to the Passport login server.
		 */
		TwnAuth 
	};

	/**
	 * Build a new TweenerHandler
	 * @param stream SecureStream instance.
	 */
	TweenerHandler(SecureStream *stream);
	/**
	 * d-tor.
	 */
	~TweenerHandler();
	
	/**
	 * @brief Setup the login information for the negotiation
	 * You must call this method before calling start().
	 * @param tweener tweener string challenge received from Messenger Notification server.
	 * @param passportId the passport id.
	 * @param password password.
	 */
	void setLoginInformation(const QString &tweener, const QString &passportId, const QString &password);

	/**
	 * @brief Return the succes of the negotiation.
	 * @return false if the passport was bad.
	 */
	bool success() const;
	/**
	 * @brief Get the ticket retrieve by this class.
	 * Call this in the slot of result() signal.
	 *
	 * @return the Passport ticket.
	 */
	QString ticket() const;

signals:
	/**
	 * Emitted when the negotiation is done.
	 * Look with success() if the task was a succes or not and
	 * ticket() to get the retrieved ticket.
	 * @param tweenerHandler this
	 */
	void result(TweenerHandler *tweenerHandler);

public slots:
	/**
	 * @brief Start negotiation process.
	 * You must set login information before or this class will fail.
	 */
	void start();

private slots:
	/**
	 * @internal
	 * We are connected to the server, send the HTTP request depending of the current state.
	 */
	void slotConnected();
	/**
	 * @internal
	 * We received a response from the server. Parse it according to the current state.
	 */
	void slotReadyRead();

private:
	/**
	 * @internal
	 * Set success and emit result signal.
	 */
	void emitResult(bool success);
	/**
	 * @internal
	 * Change server. A redirection if you prefer ;)
	 * @param host Host of the server to connect.
	 */
	void changeServer(const QString &host);

	/**
	 * @internal
	 * Send a complete HTTP request on the server.
	 * @param httpHeader HTTP header to send.
	 */
	void sendRequest(const QHttpRequestHeader &httpHeader);

	class Private;
	Private *d;
};

}

#endif
