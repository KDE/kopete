/*
    kopetemimetypehandler.h - Kopete Mime-type Handlers

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEMIMETYPEHANDLER_H
#define KOPETEMIMETYPEHANDLER_H

class KURL;
class QString;
class QStringList;

namespace Kopete
{

/**
 * @brief A handler for some set of mime-types
 * A mime type handler is responsible for handling requests to open files of
 * certain mime types presented to the main application.
 */
class MimeTypeHandler
{
protected:
	MimeTypeHandler( bool canAcceptRemoteFiles = false );
public:
	virtual ~MimeTypeHandler();

	/**
	 * Finds a MimeTypeHandler for a given URL, and tells that handler to handle it
	 *
	 * @param url the url to dispatch
	 *
	 * @return true if a handler was registered for the mime type, false otherwise
	 */
	static bool dispatchURL( const KURL &url );

	/**
	 * Returns a list of mime types this object is registered to handle
	 */
	const QStringList mimeTypes() const;

	/**
	 * Returns true if this handler can accept remote files direcltly;
	 * If false, remote files are downloaded via KIO::NetAccess before
	 * being passed to handleURL
	 */
	bool canAcceptRemoteFiles() const;

	/**
	 * Handles the URL @p url, which has the mime type @p mimeType
	 *
	 * @param mimeType The mime type of the URL
	 * @param url The url to handle
	 */
	virtual void handleURL( const QString &mimeType, const KURL &url ) const = 0;

protected:
	/**
	 * Register this object as the handler of type @p mimeType.
	 * @param mimeType the mime type to handle
	 * @return true if registration succeeded, false if another handler is
	 *         already set for this mime type.
	 */
	bool registerAsHandler( const QString &mimeType );

private:
	class Private;
	Private *d;
};

/**
 * Mime-type handler class for Kopete emoticon files
 */
class EmoticonMimeTypeHandler : public MimeTypeHandler
{
public:
	EmoticonMimeTypeHandler();

	const QStringList mimeTypes() const;

	void handleURL( const QString &mimeType, const KURL &url ) const;
};

} // Kopete

#endif
// vim: set noet ts=4 sts=4 sw=4:
