/*
    YMSGtransfer - Kopete Yahoo Protocol

    Copyright (c) 2004 Duncan Mac-Vicar Prett <duncan@kde.org>

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

#include "ymsgtransfer.h"

class YMSGTransferPrivate
{
public:
	
//	Buffer buffer;
	
	bool valid;
};

YMSGTransfer::YMSGTransfer()
{
	d = new YMSGTransferPrivate;
	d->valid = true;
}

YMSGTransfer::~YMSGTransfer()
{
	delete d;
}

Transfer::TransferType YMSGTransfer::type()
{
	return Transfer::YMSGTransfer;
}

bool YMSGTransfer::isValid()
{
	return d->valid;
}
