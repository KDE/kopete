/***************************************************************************
                          msnprotocol.h  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MSNPROTOCOL_H
#define MSNPROTOCOL_H

#include <improtocol.h>
#include <qpixmap.h>

/**
  *@author duncan
  */

class MSNProtocol : public IMProtocol  {
public: 
	MSNProtocol();
	~MSNProtocol();
	/* Plugin reimplementation */
	void init();
	bool unload();
	/** IMProtocol reimplementation */
	virtual QPixmap getProtocolIcon();
	virtual void Connect();
};

#endif
