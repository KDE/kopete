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

#include <QtCore/QList>
#include <QtCore/QPair>

class QHttpHeader;

namespace Papillon
{

/**
 * @brief HttpTransfer represent a transfer between a HTTP server.
 * This is wrapper around QHttpHeader and a body.
 * Based on QHttpHeader classes.
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
	 * @brief Get the current HttpTransfer type.
	 * @return the current HttpTransfer type
	 */
	HttpTransferType transferType() const;

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
	 * @brief Set a complete QHttpHeader.
	 * Replace the internal QHttpHeader. The pointer is handled by this class afterwards.
	 * @param header QHttpHeader to replace.
	 */
	void setHttpHeader(QHttpHeader *header);

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
