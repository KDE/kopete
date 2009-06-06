/*
    privacyguiclient.h

    Copyright (c) 2006 by Andre Duffeck        <duffeck@kde.org>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef PRIVACYGUICLIENT_H
#define PRIVACYGUICLIENT_H

#include <QObject>
#include <kxmlguiclient.h>

namespace Kopete { class ChatSession; }
class KAction;

class PrivacyGUIClient : public QObject , public KXMLGUIClient
{
Q_OBJECT
public:
	PrivacyGUIClient(Kopete::ChatSession *parent = 0);
	~PrivacyGUIClient();
private slots:
	void slotAddToWhiteList();
	void slotAddToBlackList();

private:
	Kopete::ChatSession *m_manager;
	KAction *actionAddToWhiteList;
	KAction *actionAddToBlackList;
};

#endif
