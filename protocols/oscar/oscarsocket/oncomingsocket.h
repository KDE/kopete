/***************************************************************************
                          oncomingsocket.h  -  description
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by twl6
    email                : twl6@paranoia.STUDENT.CWRU.Edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ONCOMINGSOCKET_H
#define ONCOMINGSOCKET_H

#include <qwidget.h>
#include <qserversocket.h>

/**Handles oncoming connections
  *@author twl6
  */

class OncomingSocket : public QServerSocket  {
   Q_OBJECT
public: 
	OncomingSocket(QObject *parent=0, const char *name=0);
	OncomingSocket(const QHostAddress &address, Q_UINT16 port=0,
		int backlog=5, QObject *parent=0, const char *name=0);
	~OncomingSocket();
  /** Called when someone connects to the serversocket */
  virtual void newConnection( int socket );
};

#endif
