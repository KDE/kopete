/*
   Kopete Oscar Protocol
   xtrazxservice.h - Xtraz XService

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

#ifndef XTRAZXSERVICE_H
#define XTRAZXSERVICE_H


#include <QDomDocument>
#include <QDomElement>

namespace Xtraz
{

class XService
{
public:
	enum Type { Request, Response };

	XService();
	virtual ~XService();

	/** Creates XML representation of service */
	QDomElement create( QDomDocument& doc, Type type ) const;

	/** Parses XML representation of service */
	void handle( QDomElement& eRoot );

	/** Returns service id */
	virtual QString serviceId() const;

protected:
	/** Services should implement this to create request. */
	virtual void createRequest( QDomDocument&, QDomElement & ) const {}

	/** Services should implement this to create response. */
	virtual void createResponse( QDomDocument& doc, QDomElement &e ) const;

	/** Services should implement this to handle request. */
	virtual void handleRequest( QDomElement& ) {}

	/** Services should implement this to handle response. */
	virtual void handleResponse( QDomElement& ) {}
};

}

#endif
