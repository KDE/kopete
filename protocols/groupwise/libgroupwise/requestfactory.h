//
// C++ Interface: requestfactory
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
