/*
    kopeteeventpresentation.h - Kopete Custom Notify Data Object

    Copyright (c) 2004 by Will Stephenson     <lists@stevello.free-online.co.uk>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEEVENTPRESENTATION_H
#define KOPETEEVENTPRESENTATION_H

#include <qstring.h>

#include "kopete_export.h"

namespace Kopete
{

class KOPETE_EXPORT EventPresentation
{
public:
	enum PresentationType { Sound, Message, Chat };
	EventPresentation( const PresentationType type );
	EventPresentation( const PresentationType type, 
			const QString &content = QString::null,
			const bool singleShot = false, const bool enabled = false );
	~EventPresentation();

	PresentationType type();
	QString content();
	bool enabled();
	bool singleShot();

	void setContent( const QString &content );
	void setEnabled( const bool enabled );
	void setSingleShot( const bool singleShot );
	QString toString();
private:
	PresentationType m_type;
	QString m_content;
	bool m_enabled;
	bool m_singleShot;
};

}

#endif
