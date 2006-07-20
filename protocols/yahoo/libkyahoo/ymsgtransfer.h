/*
    Kopete Yahoo Protocol
    Handles logging into to the Yahoo service

    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>

    Copyright (c) 2005 André Duffeck <andre.duffeck@kdemail.net>

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

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

#include <QByteArray>
#include <QPair>
#include <QList>

#include "libkyahoo_export.h"

class YMSGTransferPrivate;
class QString;

typedef QPair< int, QByteArray > Param;
typedef QList< Param > ParamList;

/**
@author Duncan Mac-Vicar Prett
*/
class LIBKYAHOO_EXPORT YMSGTransfer : public Transfer
{
public:
	YMSGTransfer(Yahoo::Service service);
	YMSGTransfer(Yahoo::Service service, Yahoo::Status status);
	YMSGTransfer();
	~YMSGTransfer();


	TransferType type();

	//! Get the validity of the transfer object
	bool isValid();
	Yahoo::Service service() const;
	void setService(Yahoo::Service service);
	Yahoo::Status status();
	void setStatus(Yahoo::Status status);
	unsigned int id();
	void setId(unsigned int id);

	ParamList paramList();
	QByteArray firstParam( int index );
	QByteArray nthParam( int index, int occurence );
	QByteArray nthParamSeparated( int index, int occurence, int separator );
	int paramCount( int index );
	

	void setParam(int index, const QByteArray &data);
	void setParam(int index, int data);
	QByteArray serialize();
	
	int length();
private:
	YMSGTransferPrivate* d;
};

#endif
