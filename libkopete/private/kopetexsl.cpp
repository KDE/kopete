/*
    kopetexsl.cpp - Kopete XSL Routines

    Copyright (c) 2003 by Jason Keirstead <jason@keirstead.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxml/xmlIO.h>
#include <libxml/parser.h>

#include <qtimer.h>
#include <kdebug.h>
#include <kopetexsl.h>
#include <qregexp.h>

//#define XSL_DEBUG 1

extern int xmlLoadExtDtdDefaultValue;

const QString KopeteXSL::xsltTransform( const QString &xmlString, const QString &xslString )
{
	KopeteXSLThread mThread( xmlString, xslString );
	mThread.start();
	mThread.wait();
	return mThread.result();
}

void KopeteXSL::xsltTransformAsync( const QString &xmlString, const QString &xslString,
			QObject *target, const char* slotCompleted )
{
	KopeteXSLThread *mThread = new KopeteXSLThread( xmlString, xslString );
	QObject::connect( mThread, SIGNAL(complete( const QString & )), target, slotCompleted );
	mThread->start();
}

QDomDocument KopeteXSL::xsltTransform( const QDomDocument &xmlDocument, const QDomDocument &xslDocument )
{
	KopeteXSLThread mThread( xmlDocument.toString(), xslDocument.toString() );
	mThread.start();
	mThread.wait();
	return mThread.resultDocument();
}

void KopeteXSL::xsltTransformAsync( const QDomDocument &xmlDocument, const QDomDocument &xslDocument,
			QObject *target, const char* slotCompleted )
{
	KopeteXSLThread *mThread = new KopeteXSLThread( xmlDocument.toString(), xslDocument.toString() );
	QObject::connect( mThread, SIGNAL(documentComplete( const QString & )), target, slotCompleted );
	mThread->start();
}

KopeteXSLThread::KopeteXSLThread( const QString &xmlString, const QString &xslString )
{
	m_xml = xmlString;
	m_xsl = xslString;
}

void KopeteXSLThread::run()
{
	xsltStylesheetPtr style_sheet = NULL;
	xmlDocPtr xmlDoc, xslDoc, resultDoc;

	//Init Stuff
	xmlInitMemory();
	xmlLoadExtDtdDefaultValue = 0;
	xmlSubstituteEntitiesDefault(1);

	#ifdef XSL_DEBUG
		kdDebug() << m_xml << endl;
		kdDebug() << m_xsl << endl;
	#endif

	// Convert QString into a C string
	QCString xmlCString = m_xml.latin1();
	QCString xslCString = m_xsl.latin1();

	// Read XML docs in from memory
	xmlDoc = xmlParseMemory( xmlCString, xmlCString.length() );
	xslDoc = xmlParseMemory( xslCString, xslCString.length() );

	if( xslDoc != NULL && xmlDoc != NULL )
	{
		style_sheet = xsltParseStylesheetDoc( xslDoc );
		resultDoc = xsltApplyStylesheet(style_sheet, xmlDoc, NULL);
		if( resultDoc != NULL )
		{
			//Save the result into the QString
			xmlOutputBufferPtr outp = xmlOutputBufferCreateIO( writeToQString, (xmlOutputCloseCallback)closeQString, &m_resultString, 0);
			outp->written = 0;
			xsltSaveResultTo ( outp, resultDoc, style_sheet );
			xmlOutputBufferFlush(outp);
			xmlFreeDoc(resultDoc);
		}
		else
		{
			kdDebug() << "Transformed document is null!!!" << endl;
		}
		xmlFreeDoc(xmlDoc);
		xsltFreeStylesheet(style_sheet);
	}
	else
	{
		kdDebug() << "XML/XSL Document could not be parsed!!!" << endl;
	}

	//Cleanup
	xsltCleanupGlobals();
	xmlCleanupParser();
	xmlMemoryDump();

	//Remove escaping
	//KopeteXSL::unescape( m_resultString );

	//Save the resuling DOM document
	m_result.setContent( m_resultString );

	//Signal completion
	emit( complete( m_resultString ) );
	emit( documentComplete( m_result ) );

	//Delete ourselves
	QTimer::singleShot( 500, this, SLOT( deleteLater() ) );
}

void KopeteXSL::unescape( QString &xml )
{
	xml.replace( QRegExp( QString::fromLatin1( "\"\"" ) ), QString::fromLatin1( "\"" ) );
	xml.replace( QRegExp( QString::fromLatin1( "&gt;" ) ), QString::fromLatin1( ">" ) );
	xml.replace( QRegExp( QString::fromLatin1( "&lt;" ) ), QString::fromLatin1( "<" ) );
	xml.replace( QRegExp( QString::fromLatin1( "&quot;" ) ), QString::fromLatin1( "\"" ) );
	xml.replace( QRegExp( QString::fromLatin1( "&amp;" ) ), QString::fromLatin1( "&" ) );
}

int KopeteXSLThread::writeToQString( void * context, const char * buffer, int len )
{
	QString *t = (QString*)context;
	*t += QString::fromUtf8(buffer, len);
	return len;
}

int KopeteXSLThread::closeQString( void * context )
{
	QString *t = (QString*)context;
	*t += QString::fromLatin1("\n");
	return 0;
}

#include "kopetexsl.moc"

