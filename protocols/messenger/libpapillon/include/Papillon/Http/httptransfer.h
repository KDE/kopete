/*
   httptransfer.h - HTTP transfer

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
#ifndef PAPILLON_HTTPTRANSFER_H
#define PAPILLON_HTTPTRANSFER_H

#include <Papillon/Macros>

#include <QtCore/QList>
#include <QtCore/QPair>

class QHttpHeader;
class QHttpRequestHeader;
class QHttpResponseHeader;

namespace Papillon
{

/**
 * @class HttpTransfer httptransfer.h <Papillon/Http/Transfer>
 * @brief HttpTransfer represent a transfer between a HTTP server.
 * This is wrapper around QHttpHeader and a body.
 * Based on QHttpHeader classes.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT HttpTransfer
{
public:
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
	 * @brief Get the current HttpTransfer type.
	 * @return the current HttpTransfer type
	 */
	HttpTransferType type() const;

	/**
	 * @brief Check if the HttpTransfer is valid
	 * @return true if the HttpTransfer is valid.
	 */
	bool isValid() const;

	/**
	 * @brief Get the Content-Length value.
	 * Use value in the HTTP header.
	 * @return the content length.
	 */
	uint contentLength() const;

	/**
	 * @brief Get the Content-Type for this HTTP transfer.
	 * @return
	 */
	QString contentType() const;
	/**
	 * @brief Set the Content-Type of the HTTP body.
	 * Example are text/html, text/xml, image/png
	 * @param contentType Content-Type as string.
	 */
	void setContentType(const QString &contentType);

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
	 * @brief Replace the internal HTTP header with the given HTTP header.
	 * Replace the internal header with the given QHttpRequestHeader.
	 * @param requestHeader QHttpRequestHeader to replace.
	 */
	void setHttpHeader(const QHttpRequestHeader &requestHeader);
	/**
	 * @brief Replace the internal HTTP header with the given HTTP header.
	 * Same as above, but for QHttpResponseHeader.
	 * @param responseHeader QHttpResponseHeader to replace.
	 */
	void setHttpHeader(const QHttpResponseHeader &responseHeader);

	/**
	 * @brief Check if the header contains the "Content-Length" key.
	 * @return true if the header contains "Content-Length" key.
	 */
	bool hasContentLength() const;

	/**
	 * @brief Check if the header contains the "Content-Type" key.
	 * @return true if the header contains "Content-Type" key.
	 */
	bool hasContentType() const;

	/**
	 * @brief Check if the header has the given key.
	 * @param key the given key.
	 * @return true if the given key was found.
	 */
	bool hasKey(const QString &key) const;

	/**
	 * @brief Get the header value for the given key.
	 * @param key given key.
	 * @return the value for the given key.
	 */
	QString value(const QString &key) const;

	/**
	 * @brief Get all the key: values.
	 * @return all the key: values
	 */
	QList<QPair<QString, QString> > values() const;

	/**
	 * @brief Sets the value of the entry with the key to value.
	 * If no entry with @p key exists, a new entry with the given @p key and value is created. 
	 * If an entry with the @p key already exists, the first value is discarded and replaced with the given value.
	 * @param key Key
	 * @param value value
	 */
	void setValue(const QString &key, const QString &value);

	/**
	 * @brief Sets the header entries to be the list of key value pairs in values.
	 * @param values the values.
	 */
	void setValues(const QList<QPair<QString, QString> > &values);

	/**
	 * @brief Get the status code from the HTTP Response.
	 * Only available in HttpResponse mode.
	 *
	 * @return the status code.
	 */
	int statusCode() const;

	/**
	 * @brief Get the HTTP method for the request
	 * Use it only in HttpRequest mode.
	 * @return the current method
	 */
	QString method() const;

	/**
	 * @brief Get the path used in the request.
	 * Use it only in HttpRequest mode.
	 * @return the path in the request.
	 */
	QString path() const;

	/**
	 * @brief Get the HTTP body
	 * @return the HTTP body.
	 */
	QByteArray body() const;
	/**
	 * @brief Set the HTTP body
	 * Set the Content-Length value according to the body size.
	 * @param body body to set to the transfer.
	 */
	void setBody(const QByteArray &body);
	
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
