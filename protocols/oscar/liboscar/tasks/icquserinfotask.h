/*
  Kopete Oscar Protocol
  icquserinfotask.h - SNAC 0x15 parsing for user info

  Copyright (c) 2004 Matt Rogers <mattr@kde.org>

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

#ifndef ICQUSERINFOTASK_H
#define ICQUSERINFOTASK_H

#include <qmap.h>
#include <qstring.h>

#include "icqtask.h"
#include "icquserinfo.h"

class Transfer;

/**
@author Kopete Developers
*/
class ICQUserInfoRequestTask : public ICQTask
{
Q_OBJECT
public:
	ICQUserInfoRequestTask( Task* parent );
	~ICQUserInfoRequestTask();

	enum { Long = 0, Short };
	
	void setUser( const QString& user ) { m_userToRequestFor = user; }
	void setType( unsigned int type ) { m_type = type; }
	void setInfoToRequest( unsigned int type );
	
	ICQGeneralUserInfo generalInfoFor( const QString& contact );
	ICQEmailInfo emailInfoFor( const QString& contact );
	ICQNotesInfo notesInfoFor( const QString& contact );
	ICQMoreUserInfo moreInfoFor( const QString& contact );
	ICQWorkUserInfo workInfoFor( const QString& contact );
	ICQShortInfo shortInfoFor( const QString& contact );
	ICQInterestInfo interestInfoFor( const QString& contact );
	ICQOrgAffInfo orgAffInfoFor( const QString& contact );
	
	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();

signals:
	void receivedInfoFor( const QString& contact, unsigned int type );
	
private:
	QMap<Oscar::DWORD, ICQGeneralUserInfo> m_genInfoMap;
	QMap<Oscar::DWORD, ICQEmailInfo> m_emailInfoMap;
	QMap<Oscar::DWORD, ICQNotesInfo> m_notesInfoMap;
	QMap<Oscar::DWORD, ICQMoreUserInfo> m_moreInfoMap;
	QMap<Oscar::DWORD, ICQWorkUserInfo> m_workInfoMap;
	QMap<Oscar::DWORD, ICQShortInfo> m_shortInfoMap;
	QMap<Oscar::DWORD, ICQInterestInfo> m_interestInfoMap;
	QMap<Oscar::DWORD, ICQOrgAffInfo> m_orgAffInfoMap;
	QMap<Oscar::DWORD, QString> m_contactSequenceMap;
	QMap<QString, Oscar::DWORD> m_reverseContactMap;
	unsigned int m_type;
	QString m_userToRequestFor;

};
#endif

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;
