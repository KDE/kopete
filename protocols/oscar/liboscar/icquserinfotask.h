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
	ICQMoreUserInfo moreInfoFor( const QString& contact );
	ICQWorkUserInfo workInfoFor( const QString& contact );
	QString notesInfoFor( const QString& contact );
	ICQShortInfo shortInfoFor( const QString& contact );
	ICQInterestInfo interestInfoFor( const QString& contact );
	
	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();

signals:
	void receivedInfoFor( const QString& contact, unsigned int type );
	
private:
	QMap<int, ICQGeneralUserInfo> m_genInfoMap;
	QMap<int, ICQEmailInfo> m_emailInfoMap;
	QMap<int, ICQMoreUserInfo> m_moreInfoMap;
	QMap<int, ICQWorkUserInfo> m_workInfoMap;
	QMap<int, ICQShortInfo> m_shortInfoMap;
	QMap<int, ICQInterestInfo> m_interestInfoMap;
	QMap<int, QString> m_notesInfoMap;
	QMap<int, QString> m_contactSequenceMap;
	QMap<QString, int> m_reverseContactMap;
	unsigned int m_type;
	QString m_userToRequestFor;

};
#endif

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;
