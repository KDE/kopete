/*
    cryptographyguiclient.h

    Copyright (c) 2004 by Olivier Goffart        <ogoffart@tiscalinet.be>

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

namespace Kopete { class MessageManager; }
class KToggleAction;
namespace Kopete { class MetaContact; }

/**
 *@author Olivier Goffart
 */
class CryptographyGUIClient : public QObject, public KXMLGUIClient
{
Q_OBJECT
public:
	CryptographyGUIClient(Kopete::MessageManager *parent = 0);
	~CryptographyGUIClient();

private:
	KToggleAction *m_action;
	Kopete::MetaContact *m_first;

private slots:
	void slotToggled();
};

#endif
