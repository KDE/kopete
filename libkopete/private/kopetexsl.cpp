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

#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxml/parser.h>

#include <kdebug.h>
#include "kopetexsl.h"
#include <qsignal.h>

/**
 * @author Jason Keirstead <jason@keirstead.org>
 *
 * The thread class that actually performs the XSL processing.
 * Using a thread allows for async operation.
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
		KopeteXSLThread( const QString &xmlString, const QString &xslString,
			QObject *target = 0L, const char* slotCompleted = 0L );

		/**
		 * Re implimented from QThread. Does the processing.
		 */
		virtual void run();

		/**
		 * Returns the result string
		 */
		const QString &result() { return m_resultString; };

	private:
		QString m_xml;
		QString m_xsl;
		QString m_resultString;
		QObject *m_target;
		const char* m_slotCompleted;
};


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
	xmlLoadExtDtdDefaultValue = 0;
	xmlSubstituteEntitiesDefault(1);

	// Convert QString into a C string
	QCString xmlCString = m_xml.utf8();
	QCString xslCString = m_xsl.utf8();

	// Read XML docs in from memory
	xmlDoc = xmlParseMemory( xmlCString, xmlCString.length() );
	xslDoc = xmlParseMemory( xslCString, xslCString.length() );

	if( xmlDoc != NULL )
	{
		if( xslDoc != NULL )
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
					m_resultString = QString::fromUtf8( QCString( (char*)mem, size + 1 ) );
					free(mem);
					xmlFreeDoc(resultDoc);
				}
				else
				{
					kdDebug( 14010 ) << "Transformed document is null!!!" << endl;
				}
				xsltFreeStylesheet(style_sheet);
			}
			else
			{
				kdDebug( 14010 ) << "Document is not valid XSL!!!" << endl;
				xmlFreeDoc(xslDoc);
			}
		}
		else
		{
			kdDebug( 14010 ) << "XSL Document could not be parsed!!!" << endl;
		}
		xmlFreeDoc(xmlDoc);
	}
	else
	{
		kdDebug( 14010 ) << "XML Document could not be parsed!!!" << endl;
	}

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
		else
		{
			xmlFreeDoc(xslDoc);
		}
	}

	return retVal;
}
