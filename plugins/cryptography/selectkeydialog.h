/***************************************************************************
   Copyright (c) 2002      by y0k0            <bj@altern.org>
   Copyright (c) 2007      by Charles Connell <charles@connells.org>
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

#include <kdialog.h>
#include <k3procio.h>
#include <QPixmap>

class K3ListView;
class QCheckBox;
class Q3ListViewItem;

class SelectKeyDialog : public KDialog
{
    Q_OBJECT

public:
    explicit SelectKeyDialog( QWidget *parent = 0, const char *name = 0,bool showlocal=true);
    ~SelectKeyDialog();

private slots:
    void slotReadKey(K3ProcIO*);
    void slotOk();
    void slotpreOk();
    void slotSelect(Q3ListViewItem *item);

public:
    QString getkeyID();
    QString getkeyMail();
    bool getlocal();

private:
    K3ListView *keysListpr;
    QPixmap keyPair;
    QCheckBox *local;
    K3ProcIO * fp;
};



#endif
