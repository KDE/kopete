//Code taken from KGPG 

/***************************************************************************
                          listkeys.h  -  description
                             -------------------
    begin                : Thu Jul 4 2002
    copyright            : (C) 2002 by y0k0
    email                : bj@altern.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LISTKEYS_H
#define LISTKEYS_H

#include <kdialogbase.h>

class KListView;
class QPixmap;
class QCheckBox;
class QListViewItem;

class KgpgSelKey : public KDialogBase
{
    Q_OBJECT

public:
    KgpgSelKey( QWidget *parent = 0, const char *name = 0,bool showlocal=true);
    KListView *keysListpr;
QPixmap keyPair;
QCheckBox *local;
private slots:
void slotOk();
void slotpreOk();
void slotSelect(QListViewItem *item);
public slots:
QString getkeyID();
QString getkeyMail();
bool getlocal();
};

#endif
