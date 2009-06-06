/*
    requestfactory.h - Kopete Groupwise Protocol
  
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef REQUESTFACTORY_H
#define REQUESTFACTORY_H

#include <qstring.h>

class Request;

/**
 * Factory for obtaining @ref Request instances.  
 * @author Kopete Developers
 */
class RequestFactory{
public:
	RequestFactory();
	~RequestFactory();
	
	/**
	 * Obtain a new @ref Request instance
	 * The consumer is responsible for deleting this
	 * TODO: Allow the user to provide the fields for the request in this call
	 */
	Request * request( const QString &request);
private:
	int m_nextTransaction;
};

#endif
