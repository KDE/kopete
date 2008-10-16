/*
    messageacktask.h  - Incoming OSCAR Messaging Acknowledgement Handler

    Copyright (c) 2008      by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef MESSAGEACKTASK_H
#define MESSAGEACKTASK_H

#include "task.h"

/**
 * Handles message acks.
 * @author Roman Jarosz
*/
class MessageAckTask : public Task
{
	Q_OBJECT
public:
	MessageAckTask( Task* parent );

	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );

signals:
	void messageAck( const QString& contact, uint messageId );

};

#endif

//kate: indent-mode csands; tab-width 4;
