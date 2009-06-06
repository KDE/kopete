/*
  icqcontactbase.h  -  ICQ Contact Base

  Copyright (c) 2003      by Stefan Gehn  <metz@gehn.net>
  Copyright (c) 2003      by Olivier Goffart <ogoffart@kde.org>
  Copyright (c) 2004      by Richard Smith <kde@metafoo.co.uk>
  Copyright (c) 2006      by Roman Jarosz <kedgedev@centrum.cz>
  Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#ifndef ICQCONTACTBASE_H
#define ICQCONTACTBASE_H

#include "oscarcontact.h"
#include "kopete_export.h"

/**
 * Base class for ICQ contact over Oscar protocol
 * @author Stefan Gehn
 * @author Richard Smith
 * @author Matt Rogers
 */

class OSCAR_EXPORT ICQContactBase : public OscarContact
{
Q_OBJECT

public:

	/** Normal ICQ constructor */
	ICQContactBase( Kopete::Account *account, const QString &name, Kopete::MetaContact *parent,
	            const QString& icon = QString() );
	virtual ~ICQContactBase();

private slots:
	void receivedXStatusMessage( const QString& contact, int icon, const QString& description, const QString& message );

};

#endif
//kate: tab-width 4; indent-mode csands; space-indent off; replace-tabs off;
