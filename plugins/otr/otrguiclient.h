/***************************************************************************
 *   Copyright (C) 2007 by Michael Zanetti 
 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef OTRGUICLIENT_H
#define OTRGUICLIENT_H

#include <qobject.h>
#include <kxmlguiclient.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

#include <kopetemessage.h>
#include <kopeteplugin.h>


class KActionMenu;
class KAction;

namespace Kopete { class ChatSession; }

/**
  * @author Frank Scheffold
  */




class OtrGUIClient : public QObject , public KXMLGUIClient
{
	Q_OBJECT

public:

	OtrGUIClient( Kopete::ChatSession *parent);
	~OtrGUIClient();


private:
	Kopete::ChatSession *m_manager;
	KActionMenu *otrActionMenu;
	KAction *actionEnableOtr;
	KAction *actionDisableOtr;
	KAction *actionVerifyFingerprint;

private slots:
	void slotEnableOtr();
	void slotDisableOtr();
        void encryptionEnabled( Kopete::ChatSession* session, int state );
	void slotVerifyFingerprint();
	
signals:
	void signalOtrChatsession( Kopete::ChatSession* session, bool enable );
	void signalVerifyFingerprint( Kopete::ChatSession *session );


};

#endif
