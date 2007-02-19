/*
   Kopete Oscar Protocol
   icqtask.h - SNAC 0x15 parsing 

   Copyright (c) 2004 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

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

#ifndef ICQTASK_H
#define ICQTASK_H

#include "task.h"

using namespace Oscar;

class Buffer;

class ICQTask : public Task
{
Q_OBJECT
public:
	ICQTask( Task* parent );
	~ICQTask();

	virtual void onGo();
	virtual bool forMe( const Transfer* t ) const;
	virtual bool take( Transfer* t );

	void parseInitialData( Buffer buf );
	Buffer* addInitialData( Buffer* buf = 0 ) const;

	Oscar::DWORD uin() const;
	void setUin( Oscar::DWORD uin );

	Oscar::WORD sequence() const;
	void setSequence( Oscar::WORD seqeunce );
	
	Oscar::DWORD requestType() const;
	void setRequestType( Oscar::WORD type );
	
	Oscar::DWORD requestSubType() const;
	void setRequestSubType( Oscar::WORD subType );

private:
	Oscar::DWORD m_icquin; //little endian
	Oscar::WORD m_sequence;
	Oscar::WORD m_requestType; //little endian
	Oscar::WORD m_requestSubType; //little endian
};

//kate: tab-width 4; indent-mode csands;

#endif
