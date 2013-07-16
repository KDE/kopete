/*
    cryptographyguiclient.h

    Copyright (c) 2004      by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2007      by Charles Connell        <charles@connells.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef CRYPTOGRAPHYGUICLIENT_H
#define CRYPTOGRAPHYGUICLIENT_H

#include <qobject.h>
#include <kxmlguiclient.h>
#include <ktoggleaction.h>

namespace Kopete { class ChatSession; }


/**
 *@author Olivier Goffart
 *Add functionality to a chat window
 */
class CryptographyGUIClient : public QObject, public KXMLGUIClient
{
		Q_OBJECT
	public:
		explicit CryptographyGUIClient ( Kopete::ChatSession *parent = 0 );
		~CryptographyGUIClient();

		bool signing() { return m_signAction->isChecked(); }
		bool encrypting() { return m_encAction->isChecked(); }

		KToggleAction *m_encAction;
		KToggleAction *m_signAction;
		KAction *m_exportAction;


	private slots:
		void slotEncryptToggled();
		void slotSignToggled();
		void slotExport();

};

#endif // CRYPTOGRAPHYGUICLIENT_H
