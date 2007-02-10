/*
    message.h - Peer to Peer Message class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__MESSAGE_H
#define CLASS_P2P__MESSAGE_H

#include <qstring.h>
#include <qmap.h>
#include <qvariant.h>

namespace PeerToPeer
{

/**
 * @brief Represents a session layer message.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class Message
{
	public :
		/** @brief Creates a new instance of the Message class. */
		Message(const QString& version="1.0");
		Message(const Message& other);
		Message & operator=(const Message& other);
		virtual ~Message();

		const QString body() const;
		const Q_INT32 contentLength() const;
		const QString contentType() const;
		QMap<QString, QVariant> & context();
		const QMap<QString, QVariant> & context() const;
		const Q_INT32 correlationId() const;
		// Copies the content from the specified header collection to this instance.
		void copyHeadersFrom(const QMap<QString, QVariant> & collection);
		virtual const QString from() const;
		QMap<QString, QVariant> & headers();
		const QMap<QString, QVariant> & headers() const;
		const Q_INT32 id() const;
		void setBody(const QString& body);
		void setId(const Q_INT32 id);
		void setCorrelationId(const Q_INT32 correlationId);
		virtual const QString to() const;
		const QString version() const;

	public :
		/** @brief Parses header fields from the supplied input string. */
		static void parseHeaders(const QString& input, QMap<QString, QVariant> & headers);

	private:
		class MessagePrivate;
		MessagePrivate *d;

}; // Message
}

#endif
