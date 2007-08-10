//File Imported from  KGPG  ( 2004 - 09 - 03 )

/***************************************************************************
                          popuppublic.h  -  description
                             -------------------
    begin                : Sat Jun 29 2002
    copyright            : (C) 2002 by Jean-Baptiste Mardelle
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

#include <kdialog.h>

//#include <kiconloader.h>
#include <kshortcut.h>
//Added by qt3to4:
#include <QPixmap>


class QCheckBox;
class K3ListView;
class Q3ButtonGroup;
class K3ProcIO;

class popupPublic : public KDialog
{
        Q_OBJECT
public:

        explicit popupPublic(QWidget *parent = 0, const QString& sfile=QString(), bool filemode=false,
                    const KShortcut& goDefaultKey=KShortcut(QKeySequence(Qt::CTRL+Qt::Key_Home)));
	~popupPublic();
        K3ListView *keysList;
        QCheckBox *CBarmor,*CBuntrusted,*CBshred,*CBsymmetric,*CBhideid;
        bool fmode,trusted;
        QPixmap keyPair,keySingle,keyGroup;
        QString seclist;
	QStringList untrustedList;

private:
        KConfig *config;
        Q3ButtonGroup *boutonboxoptions;
        QString customOptions;

private slots:
        void customOpts(const QString &);
        void slotprocread(K3ProcIO *);
        void slotpreselect();
        void refreshkeys();
        void refresh(bool state);
        void isSymetric(bool state);
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

