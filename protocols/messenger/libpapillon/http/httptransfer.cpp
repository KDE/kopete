/*
   httptransfer.h - HTTP transfer

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "httptransfer.h"

#include <QtNetwork/QHttpHeader>
#include <QtNetwork/QHttpRequestHeader>
#include <QtNetwork/QHttpResponseHeader>

namespace Papillon
{

class HttpTransfer::Private
{
public:
	// We use base class as a pointer for polymorphist.
	QHttpHeader *httpHeader;
};

HttpTransfer::HttpTransfer(HttpTransferType type)
 : d(new Private)
{
}

HttpTransfer::~HttpTransfer()
{
	delete d;
}

}