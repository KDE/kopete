/***************************************************************************
                          ssidata.h  -  description
                             -------------------
    begin                : Wed Aug 14 2002
    copyright            : (C) 2002 by Tom Linsky
    email                : twl6@po.cwru.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SSIDATA_H
#define SSIDATA_H

#include <qstring.h>
#include <qptrlist.h>

/**Manages SSI data from the server
  *@author Tom Linsky
  */


struct SSI { //ssi data
	QString name;
	int gid;
	int bid;
	int type;
	char *tlvlist;
	int tlvlength;
};

class SSIData : public QPtrList<SSI>
{
public: 
	SSIData();
	~SSIData();
  /** Find the group named name, and returns a pointer to it */
  SSI *findGroup(const QString &name);
  /** Adds a buddy to the SSI data list */
  SSI *addBuddy(const QString &name, const QString &group);
  /** Adds a group to the local ssi data */
  SSI *addGroup(const QString &name);
  /** Finds the buddy with given name and group... returns NULL if not found */
  SSI * findBuddy(const QString &name, const QString &group);
  /** Prints out the SSI data */
  void print(void);
};

#endif
