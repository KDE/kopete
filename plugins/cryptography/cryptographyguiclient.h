/*
    cryptographyguiclient.h

    Copyright (c) 2004 by Olivier Goffart        <ogoffart @ kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef CRYPTOGUICLIENT_H
#define CRYPTOGUICLIENT_H

#include <qobject.h>
#include <kxmlguiclient.h>

namespace Kopete { class ChatSession; }
class KToggleAction;

/**
 *@author Olivier Goffart
 */
class CryptographyGUIClient : public QObject, public KXMLGUIClient
{
Q_OBJECT
public:
	CryptographyGUIClient(Kopete::ChatSession *parent = 0);
	~CryptographyGUIClient();

private:
	KToggleAction *m_action;

private slots:
	void slotToggled();
};

#endif
