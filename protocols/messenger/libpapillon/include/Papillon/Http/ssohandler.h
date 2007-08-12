/*
	ssohandler.h : SSO handler header file

    Copyright (c) 2007		by Zhang Panyong        <pyzhang@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef PAPILLONSSOHANDLER_H
#define PAPILLONSSOHANDLER_H

#include <Papillon/Macros>
#include <QtCore/QObject>

namespace Papillon
{

/*Messenger user key structure*/
typedef struct MessengerUserKey_s
{
	unsigned int uStructHeaderSize;
	unsigned int uCryptMode;
	unsigned int uCipherType;
	unsigned int uHashType;
	unsigned int uIVLen;
	unsigned int uHashLen;
	unsigned int uCipherLen;
	unsigned char aIVBytes[8];
	unsigned char aHashBytes[20];
	unsigned char aCipherBytes[72];
}MessengerUserKey_t;

class PAPILLON_EXPORT SsoHandler : public QObject
{
	Q_OBJECT
public:
	SSOHandler::SSOHandler(HttpConnection *connection, QObject *parent);
	SSOHandler::~SSOHandler();
	void SSOHandler::setLoginInformation(const QString &ssoMethod, const QString &passportId, const QString &password);
	void SSOHandler::start();
	QString SSOHandler::getToken(QString key);
	QString SSOHandler::ticket();

public slots:
	/**
	 * @brief Start negotiation process.
	 * You must set login information before or this class will fail.
	 */
	void start();

private slots:
	/**
	 * @internal
	 * We received a response from the server. Parse it according to the current state.
	 */
	void slotConnectionReadyRead();

private:
	/**
	 * @internal
	 * Set success and emit result signal.
	 */
	void emitResult(bool success);

	class Private;
	Private *d;
}

}
#endif  /*PAPILLONSSOHANDLER_H*/

