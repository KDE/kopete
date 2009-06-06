/*
 aimcontactbase.h  -  AIM Contact Base

 Copyright (c) 2003 by Will Stephenson
 Copyright (c) 2004 by Matt Rogers <mattr@kde.org>
 Copyright (c) 2006 by Roman Jarosz <kedgedev@centrum.cz>
 Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
*/

#ifndef AIMCONTACTBASE_H
#define AIMCONTACTBASE_H

#include "oscarcontact.h"
#include "kopete_export.h"

namespace Kopete
{
class ChatSession;
}

class OSCAR_EXPORT AIMContactBase : public OscarContact
{
Q_OBJECT

public:
	AIMContactBase( Kopete::Account*, const QString&, Kopete::MetaContact*,
	            const QString& icon = QString() );
	virtual ~AIMContactBase();

	/**
	 * Gets the last time an autoresponse was sent to this contact
	 * @returns QDateTime Object that represents the date/time
	 */
	 QDateTime lastAutoResponseTime() {return m_lastAutoresponseTime;}	

	/** Sends an auto response to this contact */
	virtual void sendAutoResponse(Kopete::Message& msg);

protected:
	bool m_mobile; // Is this user mobile (i.e. do they have message forwarding on, or mobile AIM)

private:
	QDateTime m_lastAutoresponseTime;
	
};
#endif
//kate: tab-width 4; indent-mode csands;
