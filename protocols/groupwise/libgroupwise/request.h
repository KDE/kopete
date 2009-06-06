/*
    request.h - Kopete Groupwise Protocol
  
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
#ifndef LIBGW_REQUEST_H
#define LIBGW_REQUEST_H

#include "usertransfer.h"

/**
 * Represents a client generated request to the server
 * Create with @ref RequestFactory::request().
 * @author Kopete Developers
*/
class Request : public UserTransfer
{
friend class RequestFactory;

public:
	~Request( );
	QString command();
	TransferType type() { return Transfer::RequestTransfer; }
private:
	Request( const int transactionId, const QString &command );
	QString m_command;
};

#endif
