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

// Don't try to sort the libxslt includes alphabetically!
// transform.h _HAS_ to be after xsltInternals.h and xsltconfig.h _HAS_ to be
// the first libxslt include or it will break the compilation on some
// libxslt versions
#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>

// stdlib.h is required to build on Solaris
#include <stdlib.h>

#include <qregexp.h>
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
	KopeteXSLThread( const QString &xmlString, const QCString &xslString, QObject *target = 0L, const char *slotCompleted = 0L );

	/**
	 * Reimplemented from QThread. Does the processing.
	 */
	virtual void run();

	static QString xsltTransform( const QString &xmlString, const QCString &xsltString );

	/**
	 * Returns the result string
	 */
	const QString &result()
	{ return m_resultString; };

private:
	QString m_xml;
	QCString m_xsl;
	QString m_resultString;
	QObject *m_target;
	const char *m_slotCompleted;
};

KopeteXSLThread::KopeteXSLThread( const QString &xmlString, const QCString &xsltString, QObject *target, const char *slotCompleted )
{
	m_xml = xmlString;
	m_xsl = xsltString;

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

QString KopeteXSLThread::xsltTransform( const QString &xmlString, const QCString &xslCString )
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
			}
			else
			{
				errorMsg = i18n( "Document is not valid XSL!" );
				xmlFreeDoc( xslDoc );
			}
		}
		else
		{
			errorMsg = i18n( "The XSL document could not be parsed!" );
		}
		xmlFreeDoc( xmlDoc );
	}
	else
	{
		errorMsg = i18n( "XML document could not be parsed. This is likely due to an encoding problem. Please ensure you have selected the correct encoding for this contact." );
	}

	if ( resultString.isEmpty() )
	{
		resultString = i18n( "<div><b>Kopete encountered the following error while parsing a message:</b><br />%1</div>" ).arg( errorMsg );
	}

	#ifdef RAWXSL
		kdDebug(14000) << k_funcinfo << resultString << endl;
	#endif
	return resultString;
}

class KopeteXSLTPrivate
{
public:
	QCString document;
};

KopeteXSLT::KopeteXSLT( const QString &document, QObject *parent )
: QObject( parent )
{
	d = new KopeteXSLTPrivate;

	setXSLT( document );
}

KopeteXSLT::~KopeteXSLT()
{
	delete d;
}

void KopeteXSLT::setXSLT( const QString &_document )
{
	// Search for '<kopete-i18n>' elements and feed them through i18n().
	// After that replace the %VAR% variables with their proper XSLT counterpart.
	//
	// FIXME: Preprocessing the document using the QString API is fast and simple,
	//        but also error-sensitive.
	//        In fact, there are a couple of known issues with this algorithm that
	//        depend on the strings in the current styles. If these strings change
	//        they may break the parsing here.
	//
	//        The reason I'm doing it like this is because of issues with QDOM and
	//        namespaces in earlier Qt versions. When we drop Qt 3.1.x support we
	//        should probably convert this to more accurate DOM code. - Martijn
	QRegExp elementMatch( QString::fromLatin1( "<kopete-i18n>(.*)</kopete-i18n>" ) );
	elementMatch.setMinimal( true );
	QString document = _document;
	int pos;
	while ( ( pos = elementMatch.search( document ) ) != -1 )
	{
		QString orig = elementMatch.cap( 1 );
		//kdDebug( 14010 ) << k_funcinfo << "Original text: " << orig << endl;

		// Split on % and go over all parts
		// WARNING: If you change the translator comment, also change it in the
		//          styles/extracti18n Perl script, because the strings have to be
		//          identical!
		QStringList parts = QStringList::split( '%', i18n(
			"Translators: The %FOO% placeholders are variables that are substituted "
			"in the code, please leave them untranslated", orig.utf8() ), true );

		// The first part is always text, as our variables are written like %FOO%
		QStringList::Iterator it = parts.begin();
		QString trans = *it;
		bool prependPercent = true;
		it = parts.remove( it );
		for ( it = parts.begin(); it != parts.end(); ++it )
		{
			prependPercent = false;

			if ( *it == QString::fromLatin1( "TIME" ) )
			{
				trans += QString::fromLatin1( "<xsl:value-of select=\"@time\"/>" );
			}
			else if ( *it == QString::fromLatin1( "TIMESTAMP" ) )
			{
				trans += QString::fromLatin1( "<xsl:value-of select=\"@timestamp\"/>" );
			}
			else if ( *it == QString::fromLatin1( "FORMATTEDTIMESTAMP" ) )
			{
				trans += QString::fromLatin1( "<xsl:value-of select=\"@formattedTimestamp\"/>" );
			}
			else if ( *it == QString::fromLatin1( "FROM_CONTACT_DISPLAYNAME" ) )
			{
				trans += QString::fromLatin1( "<span><xsl:attribute name=\"title\">"
					"<xsl:choose>"
						"<xsl:when test='from/contact/@contactId=from/contact/contactDisplayName/@text'>"
							"<xsl:value-of disable-output-escaping=\"yes\" select=\"from/contact/metaContactDisplayName/@text\"/>"
						"</xsl:when>"
						"<xsl:otherwise>"
							"<xsl:value-of disable-output-escaping=\"yes\"	select=\"from/contact/metaContactDisplayName/@text\"/>&#160;"
							"(<xsl:value-of disable-output-escaping=\"yes\" select=\"from/contact/@contactId\"/>)"
						"</xsl:otherwise>"
					"</xsl:choose></xsl:attribute>"
					"<xsl:attribute name=\"dir\">"
					"<xsl:value-of select=\"from/contact/contactDisplayName/@dir\"/>"
					"</xsl:attribute>"
					"<xsl:value-of disable-output-escaping=\"yes\" select=\"from/contact/contactDisplayName/@text\"/></span>" );
			}
			else if ( *it == QString::fromLatin1( "TO_CONTACT_DISPLAYNAME" ) )
			{
				trans += QString::fromLatin1( "<span><xsl:attribute name=\"title\">"
					"<xsl:choose>"
						"<xsl:when test='to/contact/@contactId=from/contact/contactDisplayName/@text'>"
							"<xsl:value-of disable-output-escaping=\"yes\" select=\"to/contact/metaContactDisplayName/@text\"/>"
						"</xsl:when>"
						"<xsl:otherwise>"
							"<xsl:value-of disable-output-escaping=\"yes\" select=\"to/contact/metaContactDisplayName/@text\"/>&#160;"
							"(<xsl:value-of disable-output-escaping=\"yes\" select=\"to/contact/@contactId\"/>)"
						"</xsl:otherwise>"
					"</xsl:choose></xsl:attribute>"
					"<xsl:attribute name=\"dir\">"
					"<xsl:value-of select=\"to/contact/contactDisplayName/@dir\"/>"
					"</xsl:attribute>"
					"<xsl:value-of disable-output-escaping=\"yes\" select=\"to/contact/contactDisplayName/@text\"/></span>" );
			}
			else if ( *it == QString::fromLatin1( "FROM_METACONTACT_DISPLAYNAME" ) )
			{
				trans += QString::fromLatin1( "<span>"
				"<xsl:attribute name=\"dir\">"
				"<xsl:value-of select=\"from/contact/metaContactDisplayName/@dir\"/>"
				"</xsl:attribute>"
				"<xsl:value-of disable-output-escaping=\"yes\" select=\"from/contact/metaContactDisplayName/@text\"/></span>" );
			}
			else if ( *it == QString::fromLatin1( "TO_METACONTACT_DISPLAYNAME" ) )
			{
				trans += QString::fromLatin1( "<span>"
				"<xsl:attribute name=\"dir\">"
					"<xsl:value-of select=\"to/contact/metaContactDisplayName/@dir\"/>"
					"</xsl:attribute>"
				"<xsl:value-of disable-output-escaping=\"yes\" select=\"to/contact/metaContactDisplayName/@text\"/></span>" );
			}
			else if ( *it == QString::fromLatin1( "FROM_CONTACT_ID" ) )
			{
				trans += QString::fromLatin1( "<span><xsl:attribute name=\"title\">"
					"<xsl:value-of disable-output-escaping=\"yes\" select=\"from/contact/contactDisplayName/@text\"/></xsl:attribute>"
					"<xsl:value-of disable-output-escaping=\"yes\" select=\"from/contact/@contactId\"/></span>" );
			}
			else if ( *it == QString::fromLatin1( "TO_CONTACT_ID" ) )
			{
				trans += QString::fromLatin1( "<span><xsl:attribute name=\"title\">"
					"<xsl:value-of disable-output-escaping=\"yes\" select=\"to/contact/contactDisplayName/@text\"/></xsl:attribute>"
					"<xsl:value-of disable-output-escaping=\"yes\" select=\"to/contact/@contactId\"/></span>" );
			}
			else if ( *it == QString::fromLatin1( "BODY" ) )
			{
				trans += QString::fromLatin1( "<xsl:value-of disable-output-escaping=\"yes\" select=\"body\"/>" );
			}
			else
			{
				if ( prependPercent )
					trans += '%';
				trans += *it;
				prependPercent = true;
			}
		}
		//kdDebug( 14010 ) << k_funcinfo << "Translated text: " << trans << endl;
		// Add "<kopete-i18n>" and "</kopete-i18n>" to length, hence the '+ 27'
		document.replace( uint( pos ), orig.length() + 27, trans );
	}

	d->document = document.utf8();

	#ifdef RAWXSL
		kdDebug(14000) << k_funcinfo << d->document << endl;
	#endif
}

QString KopeteXSLT::transform( const QString &xmlString )
{
	return KopeteXSLThread::xsltTransform( xmlString, d->document );
}

void KopeteXSLT::transformAsync( const QString &xmlString, QObject *target, const char *slotCompleted )
{
	( new KopeteXSLThread( xmlString, d->document, target, slotCompleted ) )->start();
}

bool KopeteXSLT::isValid()
{
	bool retVal = false;

	// Init Stuff
	xmlLoadExtDtdDefaultValue = 0;
	xmlSubstituteEntitiesDefault( 1 );

	xmlDocPtr xslDoc = xmlParseMemory( d->document, d->document.length() );
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

#include "kopetexsl.moc"

// vim: set noet ts=4 sts=4 sw=4:

