/*
    response.h - Kopete Groupwise Protocol

    Copyright (c) 2004      SUSE Linux AG	     http://www.suse.com

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

#ifndef GW_RESPONSE_H
#define GW_RESPONSE_H

#include "usertransfer.h"

/**
 * Represents the server's reply to a client generated request
 * @author Kopete Developers
*/
class Response : public UserTransfer
{
public:
    Response(int transactionId, int resultCode, Field::FieldList fields);
    virtual ~Response();

    TransferType type() Q_DECL_OVERRIDE
    {
        return Transfer::ResponseTransfer;
    }

    int resultCode() const;
private:
    int m_resultCode;
};

#endif
