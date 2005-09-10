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
#include "kopete_export.h"

class KopeteXSLTPrivate;

namespace Kopete
{

/**
 * @author Jason Keirstead <jason@keirstead.org>
 *
 * This class provides an easy to use interface to basic
 * libxslt transformations.
 */
class KOPETE_EXPORT XSLT : public QObject
{
	Q_OBJECT

	Q_PROPERTY( Flags flags READ flags WRITE setFlags )
	Q_PROPERTY( bool isValid READ isValid )

	Q_SETS( Flags )

public:
	/**
	 * Special flags to be used during the transformation process. Passed
	 * into the engine as processing instructions.
	 */
	enum Flags
	{
 		TransformAllMessages = 1
	};

	/**
	 * Constructor.
	 *
	 * Constructs a new Kopete XSLT parser using the provided XSLT document
	 */
	XSLT( const QString &xsltDocument, QObject *parent = 0L );

	virtual ~XSLT();

	/**
	 * Set the XSLT document
	 *
	 * @return an ORed set of @ref Flags, or 0 if none
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
	bool isValid() const;

	/**
	 * @return An ORed list of Flags that the current stylesheet provides via processing instructions
	 */
	unsigned int flags() const;

	/**
	 * Sets flags to be used for the transformation.
	 *
	 * @param flags An ORed list of flags
	 */
	void setFlags( unsigned int flags );

private:
	KopeteXSLTPrivate *d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

