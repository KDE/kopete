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

#include <kdebug.h>
#include <kopetexsl.h>
#include <qregexp.h>
#include <qsignal.h>

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
	KopeteXSLThread *mThread = new KopeteXSLThread( xmlString, xslString, target, slotCompleted );
	mThread->start();
}

bool KopeteXSL::isValid( const QString &xslString )
{
	xsltStylesheetPtr style_sheet = NULL;
	xmlDocPtr xslDoc = NULL;
	bool retVal = false;

	// Convert QString into a C string
	QCString xslCString = xslString.utf8();

	xslDoc = xmlParseMemory( xslCString, xslCString.length() );

	if( xslDoc != NULL )
	{
		style_sheet = xsltParseStylesheetDoc( xslDoc );
		if( style_sheet != NULL )
		{
			retVal = true;
			xsltFreeStylesheet(style_sheet);
		}
	}

	//Cleanup
	xsltCleanupGlobals();
	xmlCleanupParser();
	xmlMemoryDump();

	return retVal;
}

KopeteXSLThread::KopeteXSLThread( const QString &xmlString, const QString &xslString, QObject *target, const char* slotCompleted )
{
	m_xml = xmlString;
	m_xsl = xslString;

	m_target = target;
	m_slotCompleted = slotCompleted;
}

void KopeteXSLThread::run()
{
	xsltStylesheetPtr style_sheet = NULL;
	xmlDocPtr xmlDoc, xslDoc, resultDoc;

	//Init Stuff
	xmlInitMemory();
	xmlLoadExtDtdDefaultValue = 0;
	xmlSubstituteEntitiesDefault(1);

	// Convert QString into a C string
	QCString xmlCString = m_xml.utf8();
	QCString xslCString = m_xsl.utf8();

	// Read XML docs in from memory
	xmlDoc = xmlParseMemory( xmlCString, xmlCString.length() );
	xslDoc = xmlParseMemory( xslCString, xslCString.length() );

	if( xslDoc != NULL && xmlDoc != NULL )
	{
		style_sheet = xsltParseStylesheetDoc( xslDoc );
		if( style_sheet != NULL )
		{
			resultDoc = xsltApplyStylesheet(style_sheet, xmlDoc, NULL);
			if( resultDoc != NULL )
			{
				//Save the result into the QString
				xmlChar *mem;
				int size;
				xmlDocDumpMemory( resultDoc, &mem, &size );
				m_resultString = QString::fromUtf8( QCString( (const char*)mem, size ) );
				delete mem;
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
			kdDebug() << "Document is not valid XSL!!!" << endl;
		}
	}
	else
	{
		kdDebug() << "XML/XSL Document could not be parsed!!!" << endl;
	}

	//Cleanup
	xsltCleanupGlobals();
	xmlCleanupParser();
	xmlMemoryDump();

	//Signal completion
	if( m_target && m_slotCompleted )
	{
		QSignal completeSignal( m_target );
		completeSignal.connect( m_target, m_slotCompleted );
		completeSignal.setValue( m_resultString );
		completeSignal.activate();

		delete this;
	}
}

void KopeteXSL::unescape( QString &xml )
{
	xml.replace( QRegExp( QString::fromLatin1( "\"\"" ) ), QString::fromLatin1( "\"" ) );
	xml.replace( QRegExp( QString::fromLatin1( "&gt;" ) ), QString::fromLatin1( ">" ) );
	xml.replace( QRegExp( QString::fromLatin1( "&lt;" ) ), QString::fromLatin1( "<" ) );
	xml.replace( QRegExp( QString::fromLatin1( "&quot;" ) ), QString::fromLatin1( "\"" ) );
	xml.replace( QRegExp( QString::fromLatin1( "&amp;" ) ), QString::fromLatin1( "&" ) );
}
