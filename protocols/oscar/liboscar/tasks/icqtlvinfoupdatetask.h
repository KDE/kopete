/*
  Kopete Oscar Protocol
  icqtlvinfoupdatetask.h - SNAC 0x15 update user info (TLV based)

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

#ifndef ICQTLVINFOUPDATETASK_H
#define ICQTLVINFOUPDATETASK_H

#include "icqtask.h"

#include "icquserinfo.h"

class Transfer;

/**
 * @author Roman Jarosz
 */
class ICQTlvInfoUpdateTask : public ICQTask
{
public:
	ICQTlvInfoUpdateTask( Task* parent );
	~ICQTlvInfoUpdateTask();

	void setInfo( const ICQFullInfo& info );

	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();

private:
	ICQFullInfo m_info;
	Oscar::DWORD m_goSequence;

};

#endif

