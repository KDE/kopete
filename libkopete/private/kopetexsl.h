/*
    kopetexsl.h - Kopete XSL Routines

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

#ifndef _KOPETE_XSLT_H
#define _KOPETE_XSLT_H

#include <qthread.h>

class QObject;
class QSignal;

/**
 * @author Jason Keirstead <jason@keirstead.org>
 *
 * This class provides an easy to use interface to basic
 * libxslt transformations. All functions are static so there
 * is no need to create instances of this class.
 */
class KopeteXSL
{
	public:
		/**
		 * Transforms the XML string using the XSL String, synchronously
		 *
		 * @param xmlString The source XML
		 * @param xslString The source XSL
		 * @return The result of the transformation
		 */
		static const QString xsltTransform( const QString &xmlString, const QString &xslString );

		/**
		 * Transforms the XML string using the XSL String, asynchronously
		 *
		 * @param xmlString The source XML
		 * @param xslString The source XSL
		 * @param target The QObject that contains the slot to be executed when processing is complete
		 * @param slotCompleted A slot that accepts a QVariant & paramater, that is the result
		 *	of the transformation
		 */
		static void xsltTransformAsync( const QString &xmlString, const QString &xslString,
			QObject *target, const char* slotCompleted );

		/**
		 * Unescapes a string, removing XML entitiy references
		 *
		 * @param xml The string you want to unescape
		 */
		static QString unescape( const QString &xml );

		/**
		 * Check if a string is a valid XSL stylesheet
		 *
		 * @param xslString The string you want to check
		 * @return If this string represents a valid XSL stylesheet
		 */
		 static bool isValid( const QString &xslString );
};

#endif
