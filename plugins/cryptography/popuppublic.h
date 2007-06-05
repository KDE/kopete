//File Imported from  KGPG  ( 2004 - 09 - 03 )

/***************************************************************************
                          popuppublic.h  -  description
                             -------------------
    begin                : Sat Jun 29 2002
    copyright            : (C) 2007 by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
                           (C) 2002 by Jean-Baptiste Mardelle
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
#ifndef POPUPPUBLIC_H
#define POPUPPUBLIC_H

#include <KDialog>
#include <KShortcut>
#include <QPixmap>

namespace Ui
{
	class PopupPublicBase;
}

class K3ProcIO;
class KConfig;

class PopupPublic : public KDialog
{
        Q_OBJECT
public:

        explicit PopupPublic(QWidget *parent = 0, const KShortcut& goDefaultKey=KShortcut(QKeySequence(Qt::CTRL+Qt::Key_Home)));
	~PopupPublic();
        bool trusted;
        QPixmap keyPair;
	QPixmap keySingle;
	QPixmap keyGroup;
        QString seclist;
	QStringList untrustedList;

private:
        KConfig *config;
        QString customOptions;
	Ui::PopupPublicBase *ui;

private slots:
        void customOpts(const QString &);
        void slotprocread(K3ProcIO *);
        void slotpreselect();
        void refreshkeys();
        void refresh(bool state);
        void sort();
        void enable();
	void slotGotoDefaultKey();
	
public slots:
void slotAccept();
void slotSetVisible();

protected slots:
virtual void slotOk();
	
signals:
        void selectedKey(const QString & ,QString,bool,bool);
	void keyListFilled();

};

#endif // POPUPPUBLIC_H

