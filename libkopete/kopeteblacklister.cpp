/*
    kopeteblacklister.cpp - Kopete BlackLister

    Copyright (c) 2004      by Roie Kerstein         <sf_kersteinroie@bezeqint.net>
    
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "kopeteblacklister.h"
#include <kconfig.h>
#include <kglobal.h>

namespace Kopete {

BlackLister::BlackLister(QString protocolId, QString accountId, QObject *parent, const char *name)
 : QObject(parent, name)
{
	KConfig *config = KGlobal::config();
	
	m_owner = accountId;
	m_protocol = protocolId;
	config->setGroup("BlackLister");
	m_blacklist = config->readListEntry( m_protocol + QString::fromLatin1("_") + m_owner );
}


BlackLister::~BlackLister()
{
	KConfig *config = KGlobal::config();
	
	config->setGroup("BlackLister");
	config->writeEntry( m_protocol + QString::fromLatin1("_") + m_owner, m_blacklist );
	config->sync();
}

bool BlackLister::isBlocked(QString &contactId)
{
	return (m_blacklist.find( contactId ) != m_blacklist.end() );
}

bool BlackLister::isBlocked(Contact *contact)
{
	QString temp = contact->contactId();

	return isBlocked(temp);
}

void BlackLister::slotAddContact(QString &contactId)
{
	if( !isBlocked(contactId) ){
		m_blacklist += contactId;
		emit contactAdded( contactId );
	}
}

void BlackLister::slotAddContact(Contact *contact)
{
	QString temp = contact->contactId();
	
	slotAddContact( temp );
}

void BlackLister::slotRemoveContact(Contact *contact)
{
	QString temp = contact->contactId();
	
	slotRemoveContact( temp );
}

void BlackLister::slotRemoveContact(QString &contactId)
{
	if( !isBlocked(contactId) ){
		m_blacklist.remove( contactId );
		emit contactRemoved( contactId );
	}
}

};
#include "kopeteblacklister.moc"
