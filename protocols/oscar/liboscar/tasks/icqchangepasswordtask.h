/*
  Kopete Oscar Protocol
  icqchangepasswordtask.h - SNAC 0x15 change password

  Copyright (c) 2006 Roman Jarosz <kedgedev@centrum.cz>

  Kopete (c) 2006 by the Kopete developers <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This library is free software; you can redistribute it and/or         *
  * modify it under the terms of the GNU Lesser General Public            *
  * License as published by the Free Software Foundation; either          *
  * version 2 of the License, or (at your option) any later version.      *
  *                                                                       *
  *************************************************************************
*/

#ifndef ICQCHANGEPASSWORDTASK_H
#define ICQCHANGEPASSWORDTASK_H

#include "icqtask.h"

class Transfer;

/**
 * @author Roman Jarosz
 */
class ICQChangePasswordTask : public ICQTask
{
public:
	ICQChangePasswordTask( Task* parent );
	~ICQChangePasswordTask();

	void setPassword( const QString& password );
	QString password() const;

	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();

private:
	QString m_password;
	Oscar::DWORD m_goSequence;

};

#endif

