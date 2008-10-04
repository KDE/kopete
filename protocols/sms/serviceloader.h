/*
  serviceloader.h  -  SMS Plugin Service Loader

  Copyright (c) 2003      by Richard Lärkäng        <nouseforaname@home.se>
  Copyright (c) 2003      by Gav Wood               <gav@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#ifndef SERVICELOADER_H
#define SERVICELOADER_H

#include "smsservice.h"
#include <qstring.h>
#include <qstringlist.h>


class ServiceLoader
{
public:
	static SMSService* loadService(const QString& name, Kopete::Account* account);
	static QStringList services();
} ;

#endif //SERVICELOADER_H
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

