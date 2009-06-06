/*
   Kopete Oscar Protocol
   xtrazxtraznotify.cpp - Xtraz XtrazNotify

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

#include "xtrazxtraznotify.h"

#include <QTextDocument>

#include <kdebug.h>

#include "oscartypes.h"
#include "oscarmessageplugin.h"
#include "buffer.h"

#include "xtrazxawayservice.h"
#include "xtrazxrandomizerservice.h"

namespace Xtraz
{

XtrazNotify::XtrazNotify()
{
	m_type = Unknown;
}

void XtrazNotify::setSenderUni( const QString& uni )
{
	m_senderUni = uni;
}

QString XtrazNotify::pluginId() const
{
	return m_pluginId;
}

XtrazNotify::Type XtrazNotify::type() const
{
	return m_type;
}

Oscar::MessagePlugin* XtrazNotify::statusResponse( int iconIndex, const QString& description, const QString& message ) const
{
	Q_ASSERT( !m_senderUni.isEmpty() );
	QList<Xtraz::XService*> serviceList;

	// Null service it's not necessary but ICQ5 sends it.
	serviceList.append( new XService() );
	
	XAwayService* awayService = new XAwayService();
	awayService->setSenderId( m_senderUni );
	awayService->setIconIndex( iconIndex );
	awayService->setDescription( description );
	awayService->setMessage( message );
	serviceList.append( awayService );

	// Randomizer service it's not necessary but ICQ5 sends it.
	serviceList.append( new XRandomizerService() );

	QString data = createResponse( "OnRemoteNotification", serviceList );
	qDeleteAll( serviceList );

	Oscar::MessagePlugin *plugin = new Oscar::MessagePlugin();
	
	plugin->setType( Oscar::MessagePlugin::XtrazScript );
	plugin->setSubTypeId( Oscar::MessagePlugin::SubScriptNotify );
	plugin->setSubTypeText( "Script Plug-in: Remote Notification Arrive" );

	Buffer buf;
	buf.addLEDBlock( data.toUtf8() );
	plugin->setData( buf.buffer() );

	return plugin;
}

Oscar::MessagePlugin* XtrazNotify::statusRequest() const
{
	Q_ASSERT( !m_senderUni.isEmpty() );
	Oscar::MessagePlugin *plugin = new Oscar::MessagePlugin();
	
	plugin->setType( Oscar::MessagePlugin::XtrazScript );
	plugin->setSubTypeId( Oscar::MessagePlugin::SubScriptNotify );
	plugin->setSubTypeText( "Script Plug-in: Remote Notification Arrive" );
	
	XAwayService awayService;
	awayService.setSenderId( m_senderUni );
	QString data = createRequest( "srvMng", &awayService );
	
	Buffer buf;
	buf.addLEDBlock( data.toUtf8() );
	plugin->setData( buf.buffer() );
	
	return plugin;
}

bool XtrazNotify::handle( Oscar::MessagePlugin* plugin )
{
	if ( !plugin )
		return false;

	Buffer buf( plugin->data() );
	QByteArray xmlBlock = buf.getLEDBlock();
	
	QDomDocument xmlDocument;
	if ( !xmlDocument.setContent( xmlBlock ) )
	{
		kWarning(OSCAR_RAW_DEBUG) << "Cannot parse xml document!";
		return false;
	}
	
	QDomElement docElement = xmlDocument.documentElement();
	if ( docElement.tagName() == "N" )
	{
		m_type = Request;
		return handleRequest( docElement );
	}
	else if ( docElement.tagName() == "NR" )
	{
		m_type = Response;
		return handleResponse( docElement );
	}
	
	return true;
}

const QList<XService*> XtrazNotify::serviceList() const
{
	return m_serviceList;
}

const XService* XtrazNotify::findService( const QString& serviceId ) const
{
	foreach( XService* service, m_serviceList )
	{
		if ( service->serviceId() == serviceId )
			return service;
	}
	return 0;
}


QString XtrazNotify::createRequest( const QString &pluginId, const XService* service ) const
{
	QString body = QString( "<N><QUERY>%1</QUERY><NOTIFY>%2</NOTIFY></N>\r\n" );
	
	QDomDocument queryDoc = xmlQuery( pluginId );
	QString query = Qt::escape( queryDoc.toString(0) );

	QDomDocument notifyDoc = xmlNotify( service );
	QString notify = Qt::escape( notifyDoc.toString( 0 ) );

	return body.arg( query ).arg( notify );
}

QString XtrazNotify::createResponse( const QString &event, const QList<XService*> &serviceList ) const
{
	QString body = QString( "<NR><RES>%1</RES></NR>\r\n" );

	QDomDocument serviceDoc = xmlRet( event, serviceList );
	QString response = Qt::escape( serviceDoc.toString(0) );

	return body.arg( response );
}

QDomDocument XtrazNotify::xmlQuery( const QString &pluginId ) const
{
	QDomDocument doc;
	
	QDomElement eRoot = doc.createElement( "Q" );
	
	QDomElement ePluginId = doc.createElement( "PluginID" );
	ePluginId.appendChild( doc.createTextNode( pluginId ) );
	
	eRoot.appendChild( ePluginId );
	
	doc.appendChild( eRoot );
	
	return doc;
}

QDomDocument XtrazNotify::xmlNotify( const XService* service ) const
{
	QDomDocument doc;
	
	QDomElement eService = service->create( doc, XService::Request );
	doc.appendChild( eService );
	
	return doc;
}

QDomDocument XtrazNotify::xmlRet( const QString &event, const QList<XService*> &serviceList ) const
{
	QDomDocument doc;
	
	QDomElement eRoot = doc.createElement( "ret" );
	eRoot.setAttribute( "event", event );
	
	foreach( XService* service, serviceList )
		eRoot.appendChild( service->create( doc, XService::Response ) );

	doc.appendChild( eRoot );

	return doc;
}


bool XtrazNotify::handleResponse( QDomElement eRoot )
{
	QDomNode childNode;
	for ( childNode = eRoot.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
	{
		QDomElement e = childNode.toElement();
		if( !e.isNull() && e.tagName() == "RES" )
		{
			QDomDocument resDocument;
			if ( !resDocument.setContent( e.text() ) )
			{
				kWarning(OSCAR_RAW_DEBUG) << "Cannot parse xml document!";
				return false;
			}

			handleRet( resDocument.documentElement() );
			return true;
		}
	}
	return false;
}


bool XtrazNotify::handleRequest( QDomElement eRoot )
{
	QDomNode childNode;
	for ( childNode = eRoot.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
	{
		QDomElement e = childNode.toElement();
		if ( !e.isNull() )
		{
			if ( e.tagName() == "NOTIFY" )
			{
				QDomDocument resDocument;
				if ( !resDocument.setContent( e.text() ) )
				{
					kWarning(OSCAR_RAW_DEBUG) << "Cannot parse xml document!";
					return false;
				}

				QDomElement resElement = resDocument.documentElement();
				XService* service = handleServiceElement( resElement );
				if ( service )
					m_serviceList.append( service );
			}
			else if ( e.tagName() == "QUERY" )
			{
				QDomDocument qDocument;
				if ( !qDocument.setContent( e.text() ) )
				{
					kWarning(OSCAR_RAW_DEBUG) << "Cannot parse xml document!";
					return false;
				}
				
				QDomElement qElement = qDocument.documentElement();
				handleQuery( qElement );
			}
		}
	}
	return false;
}

bool XtrazNotify::handleRet( QDomElement eRoot )
{
	QDomNode childNode;
	for ( childNode = eRoot.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
	{
		QDomElement e = childNode.toElement();
		if( !e.isNull() && e.tagName() == "srv" )
		{
			XService* service = handleServiceElement( e );
			if ( service )
				m_serviceList.append( service );
		}
	}
	return true;
}

bool XtrazNotify::handleQuery( QDomElement eRoot )
{
	QDomNode childNode;
	for ( childNode = eRoot.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
	{
		QDomElement e = childNode.toElement();
		if( !e.isNull() && e.tagName() == "PluginID" )
		{
			m_pluginId = e.text();
		}
	}
	return true;
}

XService* XtrazNotify::handleServiceElement( QDomElement& eRoot ) const
{
	XService* service = 0;

	QDomElement eId = eRoot.namedItem( "id" ).toElement();
	if( eId.isNull() )
		return 0;

	service = serviceFromId( eId.text() );
	if ( service )
		service->handle( eRoot );

	return service;
}

XService* XtrazNotify::serviceFromId( const QString& id ) const
{
	if ( id == "cAwaySrv" )
		return new XAwayService();
	else if ( id.isEmpty()  )
		return new XService();
	else
		return 0;
}

}
