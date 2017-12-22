/*
  Kopete Oscar Protocol
  icquserinfoupdatetask.h - SNAC 0x15 update user info

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

#ifndef ICQUSERINFOUPDATETASK_H
#define ICQUSERINFOUPDATETASK_H

#include <QList>

#include "icqtask.h"
#include "icquserinfo.h"

class Transfer;

/**
@author Kopete Developers
*/
class ICQUserInfoUpdateTask : public ICQTask
{
public:
	ICQUserInfoUpdateTask( Task* parent );
	~ICQUserInfoUpdateTask();

	void setInfo( const QList<ICQInfoBase*>& infoList );

	bool forMe( const Transfer* transfer ) const Q_DECL_OVERRIDE;
	bool take( Transfer* transfer ) Q_DECL_OVERRIDE;
	void onGo() Q_DECL_OVERRIDE;
	
private:
	QList<ICQInfoBase*> m_infoList;
	Oscar::DWORD m_goSequence;

};
#endif

