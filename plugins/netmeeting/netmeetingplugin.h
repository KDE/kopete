/*
    netmeetingplugin.h

    Copyright (c) 2003 by Olivier Goffart <ogoffart @ kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/



#ifndef NetMeetingPLUGIN_H
#define NetMeetingPLUGIN_H

#include "kopeteplugin.h"

namespace Kopete { class ChatSession; }
class MSNChatSession;
class MSNContact;
class MSNInvitation;


class NetMeetingPlugin : public Kopete::Plugin
{
	Q_OBJECT

public:
	NetMeetingPlugin( QObject *parent, const char *name, const QStringList &args );
	~NetMeetingPlugin();

private slots:
	void slotNewKMM(Kopete::ChatSession *);
	void slotPluginLoaded(Kopete::Plugin*);
	void slotInvitation(MSNInvitation*& invitation,  const QString &bodyMSG , long unsigned int cookie , MSNChatSession* msnMM , MSNContact* c );


};

#endif

