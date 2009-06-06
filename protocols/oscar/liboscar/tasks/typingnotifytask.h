/*
    typingnotifytask.h  - Send/Receive typing notifications

    Copyright (c) 2004 by Matt Rogers <mattr@kde.org>
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef _TYPINGNOTIFYTASK_H_
#define _TYPINGNOTIFYTASK_H_

#include "task.h"
#include <qstring.h>
#include "oscartypeclasses.h"

/**
 * Handles sending and receiving mini typing notifications
 * @author Matt Rogers
 */
class TypingNotifyTask : public Task
{
Q_OBJECT
public:
	enum { Finished = 0x0000, Typed = 0x0001, Begin = 0x0002 };
	
	TypingNotifyTask( Task* parent );
	~TypingNotifyTask();
	
	virtual bool forMe( const Transfer* transfer) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();
	
	void setParams( const QString & contact, int notifyType );
	
signals:
	//! somebody started typing on the other end
	void typingStarted( const QString& contact );
	
	//! somebody finished typing
	void typingFinished( const QString& contact );
	
private:
	
	//! Parse the incoming SNAC(0x04, 0x14)
	void handleNotification();
	
private:
	Oscar::WORD m_notificationType;
	QString m_contact;
};

#endif

//kate: indent-mode csands; space-indent off; replace-tabs off;
