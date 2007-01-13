/*
   httpconnection.h - HTTP connection over SSL

   Copyright (c) 2007 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <QtCore/QObject>
#include <Papillon/Macros>

namespace Papillon
{

class SecureStream;
class HttpTransfer;
/**
 * @class HttpConnection httpconnection.h <Papillon/Http/Connection>
 * @brief HTTP connection over SSL
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT HttpConnection : public QObject
{
	Q_OBJECT
public:
	HttpConnection(SecureStream *stream, QObject *parent);
	~HttpConnection();

	/**
	 * @brief Set cookie for the HTTP connection
	 *
	 * Normally you set the cookie we have received rrom Hotmail profile
	 * message on the notification server.
	 */
	void setCookie(const QString &cookie);
	
	/**
	 * @brief Get the current cookie for the HTTP connection
	 * @return the HTTP cookie
	 */
	QString cookie() const;

public slots:
	/**
	 * @brief Connect to a HTTP service.
	 * @param server Server address (DNS or IP address)
	 */
	void connectToServer(const QString &server);
	/**
	 * @brief Close HTTP connection
	 */
	void disconnectFromServer();

	/**
	 * @brief Read a new transfer from the connection.
	 *
	 * You must delete yourself the resulting HttpTransfer. It is not
	 * managed by this class.
	 *
	 * @return a HttpTransfer or null(0) if no HttpTransfer available.
	 */
	virtual HttpTransfer *read();

	/**
	 * @brief Write a new transter on the connection
	 *
	 * HttpTransfer is managed by this class so you don't need to delete it yourself.
	 *
	 * @param transfer HttpTransfer
	 */
	virtual void write(HttpTransfer *transfer);

signals:
	void connected();
	void disconnected();
	/**
	 * @brief New data is ready to be proceeded.
	 */
	void readyRead();

private slots:
	void streamReadyRead();

	void protocolOutgoingData(const QByteArray &data);
	void protocolIncomingData();

private:
	class Private;
	Private *d;
};

}

#endif
