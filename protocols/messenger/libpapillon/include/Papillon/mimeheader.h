/*
   mimeheader.h - Create/Manage a MIME header.

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
#ifndef PAPILLONMIMEHEADER_H
#define PAPILLONMIMEHEADER_H

#include <Papillon/Macros>

#include <QtCore/QSharedDataPointer>
#include <QtCore/QVariant>

namespace Papillon 
{

/**
 * @class MimeHeader mimeheader.h <Papillon/MimeHeader>
 * @brief Create/Manage a MIME header.
 * This class build a key-value associate from a MIME header.
 * Use MimeHeader::parseMimeHeader() static method to get a MimeHeader instance from raw data.
 * You must set MIME-Version yourself.
 * It doesn't support the full MIME header, only the things required for Windows Live Messenger.
 *
 * This class is implicit shared.
 * @author Michaël Larouche <larouche@kde.org>
*/
class PAPILLON_EXPORT MimeHeader
{
public:
	/**
	 * @brief Construct an empty MimeHeader.
	 */
	MimeHeader();
	/**
	 * @brief Copy constructor
	 * It doesn't create a full copy, just make a refrence.
	 * @param copy other MimeHeader
	 */
	MimeHeader(const MimeHeader &copy);
	/**
	 * d-tor
	 */
	~MimeHeader();
	/**
	 * @brief Assignment operator.
	 * Like the copy constructor, it just make a reference.
	 * @param other other MimeHeader
	 * @return this MimeHeader.
	 */
	MimeHeader &operator=(const MimeHeader &other);

	/**
	 * @brief Parse MIME header from raw data.
	 * You can pass more than the MIME header, the parser will only parse the MIME header.
	 * @param data raw data to parse the MIME header from
	 * @return MimeHeader object.
	 */
	static MimeHeader parseMimeHeader(const QString &data);

	/**
	 * @brief Does the MIME header is valid.
	 * Just test if the MimeHeader is empty.
	 * @return true if the MIME header is valid.
	 */
	bool isValid() const;

	/**
	 * @brief Check if the header has the given key.
	 * @param key Given key.
	 * @return true if the header have the given key.
	 */
	bool hasKey(const QString &key) const;

	/**
	 * @brief Get the first value for the given key
	 * @return the Value as QVariant.
	 */
	QVariant value(const QString &key) const;
	/**
	 * @brief Add or update a value for the given key.
	 * @param key key to update or add.
	 * @param value 
	 */
	void setValue(const QString &key, const QVariant &value);

	/**
	 * @brief Get the MIME version for this header.
	 * @return the MIME version as a string.
	 */
	QString mimeVersion() const;
	/**
	 * @brief Set the MIME version for this header.
	 * By default it set to 1.0.
	 * @param mimeVersion MIME version as "1.0"
	 */
	void setMimeVersion(const QString &mimeVersion = QString("1.0"));

	/**
	 * @brief Get the content type
	 * @return the content type if the key is present.
	 */
	QString contentType() const; 
	/**
	 * @brief Set the content type for the message.
	 * @param type Content type such as text/plain
	 */
	void setContentType(const QString &type);

	/**
	 * @brief Get the charset.
	 * @return the charset as a string.
	 */
	QString charset() const;
	/**
	 * @brief Set the charset for the message.
	 * @param charset charset as a string.
	 */
	void setCharset(const QString &charset);

	/**
	 * @brief Get a string represention of the MIME header.
	 * @return a complete string looking like "Key: Value\r\n"
	 */
	QString toString() const;

private:
	class Private;
	QSharedDataPointer<Private> d;
};

}

#endif
