/*
   Kopete Oscar Protocol
   xtrazxtraznotify.h - Xtraz XtrazNotify

   Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>

   Kopete (c) 2007 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#ifndef XTRAZXTRAZNOTIFY_H
#define XTRAZXTRAZNOTIFY_H

#include <QDomDocument>
#include <QList>

namespace Oscar {
class MessagePlugin;
}

namespace Xtraz
{

class XService;

class XtrazNotify
{
public:
	enum Type { Unknown, Request, Response };
	XtrazNotify();

	/**
	 * Sets sender UNI.
	 */
	void setSenderUni( const QString& uni );

	/**
	 * Returns plugin text id.
	 */
	QString pluginId() const;

	/**
	 * Returns plugin type.
	 */
	Type type() const;

	/**
	 * Creates message plugin with status response.
	 * Sender UNI must be set before calling this function.
	 */
	Oscar::MessagePlugin* statusResponse( int iconIndex, const QString& description, const QString& message ) const;

	/**
	 * Creates message plugin with status request.
	 * Sender UNI must be set before calling this function.
	 */
	Oscar::MessagePlugin* statusRequest() const;

	/**
	 * Parses message plugin and creates appropriate XService objects.
	 */
	bool handle( Oscar::MessagePlugin* plugin );

	/**
	 * Returns list of XService objects.
	 */
	const QList<XService*> serviceList() const;

	/**
	 * Finds XService object with given serviceId.
	 */
	const XService* findService( const QString& serviceId ) const;
	
private:
	QString createRequest( const QString &pluginId, const XService* service ) const;
	QString createResponse( const QString &event, const QList<XService*> &serviceList ) const;

	QDomDocument xmlQuery( const QString &pluginId ) const;
	QDomDocument xmlNotify( const XService* service ) const;
	QDomDocument xmlRet( const QString &event, const QList<XService*> &serviceList ) const;

	bool handleRequest( QDomElement eRoot );
	bool handleResponse( QDomElement eRoot );
	bool handleRet( QDomElement eRoot );
	bool handleQuery( QDomElement eRoot );

	XService* handleServiceElement( QDomElement& eRoot ) const;
	XService* serviceFromId( const QString& id ) const;

	QList<XService*> m_serviceList;
	QString m_senderUni;
	QString m_pluginId;
	Type m_type;
};

}

#endif
