/*
   securestream.h - Secure stream based on TLS/SSL.

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
#ifndef SECURESTREAM_H
#define SECURESTREAM_H

#include <QtCore/QObject>
#include <Papillon/Macros>

namespace Papillon
{

class Connector;

/**
 * @class SecureStream securestream.h <Papillon/Http/SecureStream>
 * @brief Secure stream using TLS/SSL (Transport Layer Security/Secure Socket Layer)
 * This is used in Messenger context to retrieve MS Passport ticket for login, and retrieve Contact List from a Web Service.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT SecureStream : public QObject
{
	Q_OBJECT
public:
	/**
	 * Enum used to identity the current error that occurred.
	 */
	enum ErrorCode 
	{
		/**
		 * Nothing is wrong. (so all is correct)
		 */
		ErrorNone,
		/**
		 * TLS/SSL Handshake failed.
		 */
		ErrorHandshakeFailed,
		/**
		 * We have a valid certificate.
		 */
		ErrorValidCertificate,
		/**
		 * We have a wrong certificate (host doesn't match).
		 */
		ErrorWrongCertificate,
		/**
		 * Invalid certificate
		 */
		ErrorInvalidCertificate,
		/**
		 *  We don't have any certificate.
		 */
		ErrorNoCertificate,
		/**
		 * We got disconnected.
		 */
		ErrorDisconnected,
		/**
		 * Unknow error.
		 */
		ErrorUnknown 
	};

	/**
	 * @brief Construct a new Secure stream.
	 * @param connector the Connector to use
	 */
	SecureStream(Connector *connector);
	/**
	 * d-tor (duh)
	 */
	~SecureStream();

	/**
	 * @brief Connect to a server.
	 * No need to pass a port argument since it's always on port 443.
	 */
	void connectToServer(const QString &server);
	/**
	 * @brief Disconnect from server.
	 */
	void disconnectFromServer();

	/**
	 * @brief Return a error code.
	 * Use this error code if you want to use your own error message.
	 *
	 * @return a value of the ErrorCode enum.
	 */
	ErrorCode errorCode() const;
	/**
	 * @brief Return a English human readable of the error.
	 * The error string is not translated. Use errorCode() instead
	 * and use your custom error strings.
	 *
	 * @return a English human readable of the error.
	 */
	QString errorString() const;

public slots:
	/**
	 * Get the current de-encrypted read buffer.
	 * Call this when you receive readyRead() signal.
	 * @return Read buffer.
	 */
	QByteArray read();
	/**
	 * Write data on the secure stream.
	 * @param data data to be written.
	 */
	void write(const QByteArray &data);

signals:
	/**
	 * @brief SecureStream is connected and ready to be used.
	 * Emitted then the TLS handshake is done.
	 */
	void connected();
	/**
	 * @brief SecureStream got disconnected.
	 * Either the stream or the TLS handled disconnected.
	 */
	void disconnected();
	/**
	 * @brief Data is ready to be read from SecureStream
	 * After received this signal, use read() to get the current buffer.
	 */
	void readyRead();
	/**
	 * @brief An error occurred.
	 * Use errorCode() and errorString() to find out what happened.
	 */
	void error();
	
private slots:
	/**
	 * @internal
	 * Initialise the TLS client when the stream is connected.
	 */
	void streamConnected();
	/**
	 * @internal
	 * Data is ready to be read from the stream and be proceeded by TLS.
	 */
	void streamReadyRead();

	/**
	 * @internal
	 * TLS handshake is done.
	 * Emit connected() signal if the TLS certificate is valid.
	 */
	void tlsHandshaken();
	/**
	 * @internal
	 * TLS has finished proceeding of incoming data. Data is now ready to be used.
	 * Emit readyRead() signal.
	 */
	void tlsReadyRead();
	/**
	 * @internal
	 * TLS has finished proceeding the outgoing data. Data will be written on the stream.
	 */
	void tlsReadyWrite();
	/**
	 * @internal
	 * An error occurred in TLS.
	 */
	void tlsError();

	/**
	 * @internal
	 * Set the error to Disconnected and emit disconnected() signal.
	 */
	void slotDisconnected();
	
private:
	class Private;
	Private *d;
};

}

#endif
