/*
    kopetexsl.cpp - Kopete XSL Routines

    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetexsl.h"

#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxslt/transform.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/xsltconfig.h>

// Solaris Fix
// FIXME: _why_ is including stdlib.h a Solaris fix? Stuff like this should
//        be documented - Martijn
#include <stdlib.h>

#include <qsignal.h>
#include <qthread.h>

#include <kdebug.h>
#include <klocale.h>

/**
 * @author Jason Keirstead <jason@keirstead.org>
 *
 * The thread class that actually performs the XSL processing.
 * Using a thread allows async operation.
 */
class KopeteXSLThread : public QThread
{
public:
	/**
	 * Thread constructor
	 *
	 * @param xmlString The XML to be transformed
	 * @param xslString The XSL string we will use to transform
	 * @param target Target object to connect to for async operation
	 * @param slotCompleted Slot to fire on completion in asnc operation
	 */
	KopeteXSLThread( const QString &xmlString, const QString &xslString, QObject *target = 0L, const char *slotCompleted = 0L );

	/**
	 * Reimplemented from QThread. Does the processing.
	 */
	virtual void run();

	static QString xsltTransform( const QString &xmlString, const QString &xslString );

	/**
	 * Returns the result string
	 */
	const QString &result()
	{ return m_resultString; };

private:
	QString m_xml;
	QString m_xsl;
	QString m_resultString;
	QObject *m_target;
	const char *m_slotCompleted;
};

KopeteXSLThread::KopeteXSLThread( const QString &xmlString, const QString &xslString, QObject *target, const char *slotCompleted )
{
	m_xml = xmlString;
	m_xsl = xslString;

	m_target = target;
	m_slotCompleted = slotCompleted;
}

void KopeteXSLThread::run()
{
	m_resultString = xsltTransform( m_xml, m_xsl );

	// Signal completion
	if( m_target && m_slotCompleted )
	{
		QSignal completeSignal( m_target );
		completeSignal.connect( m_target, m_slotCompleted );
		completeSignal.setValue( m_resultString );
		completeSignal.activate();

		// FIXME: Why no 'delete this' if there's no slotCompleted? - Martijn
		delete this;
	}
}

QString KopeteXSLThread::xsltTransform( const QString &xmlString, const QString &xslString )
{
	// Init Stuff
	xmlLoadExtDtdDefaultValue = 0;
	xmlSubstituteEntitiesDefault( 1 );

	// Convert QString into a C string
	QCString xmlCString = xmlString.utf8();

	QString resultString;
	QString errorMsg;

	// Read XML docs in from memory
	xmlDocPtr xmlDoc = xmlParseMemory( xmlCString, xmlCString.length() );
	if ( xmlDoc )
	{
	    QCString xslCString = xslString.utf8();
		xmlDocPtr xslDoc = xmlParseMemory( xslCString, xslCString.length() );
		if ( xslDoc )
		{
			xsltStylesheetPtr styleSheet = xsltParseStylesheetDoc( xslDoc );
			if ( styleSheet )
			{
				xmlDocPtr resultDoc = xsltApplyStylesheet( styleSheet, xmlDoc, NULL );
				if ( resultDoc )
				{
					// Save the result into the QString
					xmlChar *mem;
					int size;
					xmlDocDumpMemory( resultDoc, &mem, &size );
					resultString = QString::fromUtf8( QCString( ( char * )( mem ), size + 1 ) );
					free( mem );
					xmlFreeDoc( resultDoc );
				}
				else
				{
					errorMsg = i18n( "Transformed document is null!" );
				}
				xsltFreeStylesheet( styleSheet );

				// FIXME: No xmlFreeDoc( xslDoc ) here? - Martijn
			}
			else
			{
				errorMsg = i18n( "Document is not valid XSL!" );
				xmlFreeDoc( xslDoc );
			}
		}
		else
		{
			errorMsg = i18n( "XSL document could not be parsed!" );
		}
		xmlFreeDoc( xmlDoc );
	}
	else
	{
		errorMsg = i18n( "XML document could not be parsed!" );
	}

	if ( resultString.isEmpty() )
		resultString = i18n( "<div><b>An internal Kopete error occurred while parsing a message:</b><br />%1</div>" ).arg( errorMsg );

	return resultString;
}

const QString KopeteXSL::xsltTransform( const QString &xmlString, const QString &xslString )
{
	return KopeteXSLThread::xsltTransform(xmlString, xslString);
}

void KopeteXSL::xsltTransformAsync( const QString &xmlString, const QString &xslString, QObject *target, const char *slotCompleted )
{
	KopeteXSLThread *mThread = new KopeteXSLThread( xmlString, xslString, target, slotCompleted );
	mThread->start();
}

bool KopeteXSL::isValid( const QString &xslString )
{
	bool retVal = false;

	// Init Stuff
	xmlLoadExtDtdDefaultValue = 0;
	xmlSubstituteEntitiesDefault( 1 );

	QCString xslCString = xslString.utf8();
	xmlDocPtr xslDoc = xmlParseMemory( xslCString, xslCString.length() );
	if ( xslDoc )
	{
		xsltStylesheetPtr styleSheet = xsltParseStylesheetDoc( xslDoc );
		if ( styleSheet )
		{
			retVal = true;
			xsltFreeStylesheet( styleSheet );
		}
		else
		{
			xmlFreeDoc( xslDoc );
		}
	}

	return retVal;
}

// vim: set noet ts=4 sts=4 sw=4:

