/*
    configmodule.h - Kopete Plugin Config Module

	Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett       <duncan@kde.org>

	Portions of this code based in Noatun plugin code:
    Copyright (c) 2000-2002 The Noatun Developers

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

#ifndef CONFIGMODULE_H
#define CONFIGMODULE_H

#include <qwidget.h>
#include <qptrlist.h>

/**
  * @author Duncan Mac-Vicar Prett <duncan@kde.org>
  *
  */

class ConfigModule : public QWidget
{
   Q_OBJECT
public:
	/**
	 * arguments are short and long descriptions
	 * for this module, respectively
	 *
	 * parent is the object that is this modules virtual-parent.
	 * When that is deleted, this also will go away, automagically.
	 **/
	ConfigModule(const QString &name, const QString &description, QObject *parent=0);
	ConfigModule(const QString &name, const QString &description, const QString &icon, QObject *parent=0);
	virtual ~ConfigModule();

public slots:
	/**
	 * save all your options, and apply them
	 **/
	virtual void save() {}
	/**
	 * reload all options (e.g., read config files)
	 **/
	virtual void reopen() {}


	/**
	 *  show the config page
	 */
	void activate();


};

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

