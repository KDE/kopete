/*  *************************************************************************
    *   copyright: (C) 2003 Richard Lärkäng <nouseforaname@home.se>         *
    *   copyright: (C) 2003 Gav Wood <gav@kde.org>                          *
    *************************************************************************
*/

/*  *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef SMSPREFERENCES_H
#define SMSPREFERENCES_H

#include "configmodule.h"

class QVBoxLayout;
class SMSService;
class SMSPrefsUI;

class SMSPreferences : public ConfigModule
{
	Q_OBJECT
public:
	SMSPreferences( const QString &pixmap, QObject *parent = 0 );
	~SMSPreferences();

	virtual void save();

signals:
	void saved();

private:
	SMSPrefsUI *theUI;
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

