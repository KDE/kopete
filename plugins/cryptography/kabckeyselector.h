/*
    kabckeyselector.cpp  -  description

    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/
#ifndef KABCKEYSELECTOR_H
#define KABCKEYSELECTOR_H

class QString;
class QStringList;
class QWidget;

/**
Show dialog about KABC key selection; different dialog depending on singular/multiple keys

	@author Charles Connell <charles@connells.org>
*/

QString KabcKeySelector ( QString displayName, QString addresseeName, QStringList keys, QWidget *parent );


#endif
