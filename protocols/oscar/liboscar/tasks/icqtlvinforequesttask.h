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
	ICQTlvInfoRequestTask( Task* parent );
	~ICQTlvInfoRequestTask();

	void setUser( const QString& contactId ) { m_userToRequestFor = contactId; }
	void setMetaInfoId( const QByteArray& id ) { m_metaInfoId = id; }

	ICQFullInfo fullInfoFor( const QString& contact );

	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();

Q_SIGNALS:
	void receivedInfoFor( const QString& contact );

private:
	void parse( const QByteArray &data );

	QMap<QString, ICQFullInfo> m_fullInfoMap;
	QMap<int, QString> m_contactSequenceMap;

	QString m_userToRequestFor;

	QByteArray m_metaInfoId;
	Oscar::WORD m_goSequence;

};

#endif

