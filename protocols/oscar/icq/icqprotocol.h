/*
  oscarprotocol.h  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

  Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#ifndef ICQOSCARPROTOCOL_H
#define ICQOSCARPROTOCOL_H

#include <qwidget.h>
#include <qmap.h>

#include "oscarprotocol.h"


class ICQProtocol : public OscarProtocol
{
	Q_OBJECT

	public:
		ICQProtocol(QObject *parent, const char *name, const QStringList &args);

	/**
	* Return the active instance of the protocol
	* because it's a singleton,
	*/
	static ICQProtocol *protocol();

	private:
		/** The active instance of oscarprotocol */
	static ICQProtocol *protocolStatic_;

};




#endif
// vim: set noet ts=4 sts=4 sw=4:
