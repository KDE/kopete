/*
	fetchaddressbook.h: Header file for Job to fetch Address Book 
	from Windows Live Messenger server using SOAP

    Copyright (c) 2007		by Zhang Panyong	        <pyzhang@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers	<kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef FETCHADDRESSBOOK_H
#define FETCHADDRESSBOOK_H

#include <Qtcore/QObject>

namespace Papillon
{

namespace Internal
{
	class FindABResult;
}

class FetchABJob: public QObject
{
	Q_OBJECT
	public:
		FetchABJob();
		~FetchABJob();

public slots:
	void execute();
signals:
	void finished(Papillon::FetchABJob *job);

private slots:
	void bindingFindAddressBookResult(Papillon::Internal::FindABResult *result);

private:
	class Private;
	private *d;
}

}

#endif/* FETCHADDRESSBOOK_H*/
