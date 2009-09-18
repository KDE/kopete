/*
  smsservice.cpp  -  SMS Plugin Service

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

#include "smsservice.h"
#include <qlayout.h>
#include <kdebug.h>

SMSService::SMSService(Kopete::Account* account)
	: QObject(), m_account(account), m_layout(NULL), m_parent(NULL)
{
}

SMSService::~SMSService()
{

}

void SMSService::setAccount(Kopete::Account* account)
{
	if(!m_account)
		m_account = account;
	if(account)
		savePreferences();
}

// The Default impl simply flips a signal back
void SMSService::connect()
{
	emit connected();
}

// The Default impl simply flips a signal back
void SMSService::disconnect()
{
	emit disconnected();
}

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:


#include "smsservice.moc"
