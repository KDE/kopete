/*
    kopetexslt.h - Kopete XSLT Routines

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

#ifndef _KOPETE_XSLT_H
#define _KOPETE_XSLT_H

#include <qobject.h>

class KopeteXSLTPrivate;

/**
 * @author Jason Keirstead <jason@keirstead.org>
 *
 * This class provides an easy to use interface to basic
 * libxslt transformations.
 */
class KopeteXSLT : public QObject
{
	Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * Constructs a new Kopete XSLT parser using the provided XSLT document
	 */
	KopeteXSLT( const QString &xsltDocument, QObject *parent = 0L );

	virtual ~KopeteXSLT();

	/**
	 * Set the XSLT document
	 */
	void setXSLT( const QString &document );

	/**
	 * Transforms the XML string using the XSLT document, synchronously
	 *
	 * @param xmlString The source XML
	 * @return The result of the transformation
	 */
	QString transform( const QString &xmlString );

	/**
	 * Transforms the XML string using the XSLT document, asynchronously
	 *
	 * @param xmlString The source XML
	 * @param target The QObject that contains the slot to be executed when processing is complete
	 * @param slotCompleted A slot that accepts a QVariant & paramater, that is the result
	 * of the transformation
	 */
	void transformAsync( const QString &xmlString, QObject *target, const char *slotCompleted );

	/**
	 * Check whether the XSLT document is valid
	 *
	 * @return Whether the document represents a valid XSLT stylesheet
	 */
	bool isValid();

private:
	KopeteXSLTPrivate *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

