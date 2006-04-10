/*
   httptransfer.h - HTTP transfer

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLON_HTTPTRANSFER_H
#define PAPILLON_HTTPTRANSFER_H

#include <papillon_macros.h>

namespace Papillon
{

/**
 * @brief HttpTransfer represent a transfer between a HTTP server.
 * Contains the HTTP header plus the data.
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class PAPILLON_EXPORT HttpTransfer
{
	/**
	 * @brief Determine the Transfer type.
	 * HttpRequest: a HTTP request (with method GET or POST)
	 * HttpResponse: a HTTP response (with a status code)
	 */
	enum HttpTransferType
	{
		HttpRequest,
		HttpResponse
	};

public:
	/**
	 * @brief Build a new HttpTransfer.
	 * By default it is set in HTTP request mode, because this is the kind of 
	 * HttpTransfer you create manually.
	 * @param type Type of HttpTransfer, by default HttpRequest.
	 */
	HttpTransfer(HttpTransferType type = HttpRequest);
	/**
	 * d-tor (duh)
	 */
	~HttpTransfer();

	/**
	 * @brief Set the request details.
	 * Only call this method in HttpRequest method.
	 * This is a wrapper around QHttpRequestHeader::setRequest().
	 *
	 * @param method HTTP method, either GET or POST
	 * @param path path to request on server.
	 * @param majorVer HTTP major version (default to 1)
	 * @param minorVer HTTP minir version (default to 1)
	 */
	void setRequest(const QString &method, const QString &path, int majorVer = 1, int minorVer = 1);

	/**
	 * @brief Return the current transfer as a raw command.
	 * It formats the HttpTransfer to be ready to be sent on a ByteStream.
	 */
	QByteArray toRawCommand();

private:
	class Private;
	Private *d;
};

}
#endif
