/*
    YMSG - Yahoo Protocol

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

#ifndef YMSG_TRANSFER_H
#define YMSG_TRANSFER_H

#include "transfer.h"

#include "yahootypes.h"
#include <qcstring.h>

class YMSGTransferPrivate;
class QString;

/**
@author Duncan Mac-Vicar Prett
*/
class YMSGTransfer : public Transfer
{
public:
	YMSGTransfer(Yahoo::Service service);
	YMSGTransfer();
	~YMSGTransfer();


	TransferType type();

	//! Get the validity of the transfer object
	bool isValid();
	Yahoo::Service service();
	void setService(Yahoo::Service service);
	Yahoo::Status status();
	unsigned int id();
	QString param(int index);
	void setParam(int index, QString data);
	QByteArray serialize();
	
	int length();
private:
	YMSGTransferPrivate* d;
};

#endif
