/*
   ircguiclient.h

    Copyright (c) 2003 by Jason Keirstead        <jason@keirstead.org>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef IRCGUICLIENT_H
#define IRCGUICLIENT_H

#include <qobject.h>
#include <kxmlguiclient.h>

namespace Kopete { class ChatSession; }
class IRCContact;

/**
 *@author Jason Keirstead
 */
class IRCGUIClient : public QObject , public KXMLGUIClient
{
	Q_OBJECT
	public:
		IRCGUIClient( Kopete::ChatSession *parent = 0 );
		~IRCGUIClient();
	
	private slots:
		void slotSelectCodec( const QTextCodec *codec );
		
	private:
		IRCContact *m_user;
};

#endif
