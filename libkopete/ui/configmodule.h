/***************************************************************************
                          Kopete Instant Messenger
							 configmodule.h
                            -------------------
				(C) 2001-2002 by Duncan Mac-Vicar P. <duncan@kde.org>
				Portions of the code,
				(C) 2001-2002 The Noatun Developers
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIGMODULE_H
#define CONFIGMODULE_H

#include <qwidget.h>
#include <qlist.h>
/**
  *@author duncan
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
	ConfigModule(const QString &name, const QString &description, const QPixmap &pixmap, QObject *parent=0);
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

private slots:
	void ownerDeleted();

};

#endif
