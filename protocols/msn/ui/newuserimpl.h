/***************************************************************************
                          newuserimpl.h  -  description
                             -------------------
    begin                : Fri Jan 5 2001
    copyright            : (C) 2001 by Olaf Lueg
    email                : olaf.lueg@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by  the Free Software Foundation; either version 2 of the   *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/
#ifndef NEWUSERIMPL_H
#define NEWUSERIMPL_H

#include <qwidget.h>

#include "newuser.h"

/**
 * @author Olaf Lueg
 */
class NewUserImpl : public NewUser  {
   Q_OBJECT
public: 
	NewUserImpl(QWidget *parent=0, const char *name=0);
	~NewUserImpl();
  /**  */
  void setHandle(const QString &_handle, const QString &_public=QString::null);
public slots: // Public slots
  /**  */
  void slotClose();
signals: // Signals
  /**  */
  void addUser( const QString & , const QString &);
  void blockUser(const QString & );
  
 private:
 QString userHandle;
 QString publicName;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

