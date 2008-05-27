/*
  oscarmyselfcontact.h  -  Oscar Protocol Plugin Myself Contact

  Copyright (c) 2004 by Richard Smith               <kde@metafoo.co.uk>
  Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#ifndef OSCARMYSELFCONTACT_H
#define OSCARMYSELFCONTACT_H

#include "kopetecontact.h"
#include "userdetails.h"
#include "kopete_export.h"

namespace Kopete
{
class ChatSession;
}

class OscarAccount;

/**
 * myself() contact for oscar protocol
 * @author Richard Smith
 */
class OSCAR_EXPORT OscarMyselfContact : public Kopete::Contact
{
Q_OBJECT

public:
	OscarMyselfContact( OscarAccount* account );
	virtual ~OscarMyselfContact();
	
	virtual bool isReachable();
	virtual Kopete::ChatSession *manager( CanCreateFlags canCreate );
	
	UserDetails details();
	
public slots:
	/** our user info has been updated */
	virtual void userInfoUpdated() = 0;
	
	/** I'm sorry Dave, I can't let you do that... */
	virtual void deleteContact();
};

#endif
//kate: tab-width 4; indent-mode csands;
