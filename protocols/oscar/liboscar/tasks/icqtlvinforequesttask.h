/*
  Kopete Oscar Protocol
  icqtlvinforequesttask.h - SNAC 0x15 parsing for full user info (TLV based)

  Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>

  Kopete (c) 2007 by the Kopete developers <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This library is free software; you can redistribute it and/or         *
  * modify it under the terms of the GNU Lesser General Public            *
  * License as published by the Free Software Foundation; either          *
  * version 2 of the License, or (at your option) any later version.      *
  *                                                                       *
  *************************************************************************
*/

#ifndef ICQTLVINFOREQUESTTASK_H
#define ICQTLVINFOREQUESTTASK_H

#include "icqtask.h"
#include "icquserinfo.h"

#include <QtCore/QMap>

class Transfer;

/**
 * @author Roman Jarosz
 */
class ICQTlvInfoRequestTask : public ICQTask
{
	Q_OBJECT
public:
	// Short TLVs: 60,85,150,160,170,180,190,200,250,260,280,390,400,410,420,430,440,450,460,470,480,490,500,505,510,520,530,540,550,560
	// Medium TLVs: Short + 50,70,85,100,110,120,130,140,300,310,320,330,340,350,360,370,380
	// Long TLVs: Medium + 270,290,291,292

	enum InfoType { Short = 0x0001, Medium = 0x0002, Long = 0x0003 };
	ICQTlvInfoRequestTask( Task* parent );
	~ICQTlvInfoRequestTask();

	void setUser( const QString& contactId ) { m_userToRequestFor = contactId; }
	void setMetaInfoId( const QByteArray& id ) { m_metaInfoId = id; }
	void setType( InfoType type ) { m_type = type; }

	ICQFullInfo fullInfoFor( const QString& contact );

	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();

Q_SIGNALS:
	void receivedInfoFor( const QString& contact );

private:
	void parse( Oscar::DWORD seq, const QByteArray &data );

	QMap<QString, ICQFullInfo> m_fullInfoMap;
	QMap<int, QString> m_contactSequenceMap;

	QString m_userToRequestFor;
	InfoType m_type;

	QByteArray m_metaInfoId;

};

#endif

