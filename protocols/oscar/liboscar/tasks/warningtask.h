/*
	Kopete Oscar Protocol
	warningtask.cpp - send warnings to aim users

	Copyright (c) 2005 by Matt Rogers <mattr@kde.org>

	Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

	*************************************************************************
	*                                                                       *
	* This library is free software; you can redistribute it and/or         *
	* modify it under the terms of the GNU Lesser General Public            *
	* License as published by the Free Software Foundation; either          *
	* version 2 of the License, or (at your option) any later version.      *
	*                                                                       *
	*************************************************************************
*/

#ifndef WARNINGTASK_H
#define WARNINGTASK_H

#include "task.h"
#include <qmap.h>
#include "oscartypes.h"

/**
@author Matt Rogers
*/
class WarningTask : public Task
{
Q_OBJECT
public:
	WarningTask( Task* parent );
	~WarningTask();
	
	void setContact( const QString& contact );
	void setAnonymous( bool anon );
	
	Oscar::WORD levelIncrease();
	Oscar::WORD newLevel();
	
	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();

signals:
	void userWarned( const QString&, quint16, quint16 );
	
private:
	QString m_contact;
	bool m_sendAnon;
	Oscar::WORD m_sequence;
	Oscar::WORD m_increase;
	Oscar::WORD m_newLevel;
};

#endif

//kate: indent-mode csands; space-indent off; replace-tabs off; tab-width 4;
