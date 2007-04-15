//Code from KGPG

/***************************************************************************
                          listkeys.cpp  -  description
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

////////////////////////////////////////////////////// code for the key management

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <qlayout.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3ListViewItem>

#include <k3listview.h>
#include <klocale.h>
#include <qcheckbox.h>
#include <k3process.h>
#include <k3procio.h>
#include <kiconloader.h>
#include <kicon.h>

#include "kgpgselkey.h"

#include <cstdio>

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////   Secret key selection dialog, used when user wants to sign a key
KgpgSelKey::KgpgSelKey(QWidget *parent, const char *name,bool showlocal)
 : KDialog( parent )
{
  setCaption( i18n("Private Key List") );
  setButtons( KDialog::Ok | KDialog::Cancel );

  QString keyname;
  QWidget *page = new QWidget(this);
  QLabel *labeltxt;
  KIconLoader *loader = KIconLoader::global();

  keyPair=loader->loadIcon("kgpg_key2",K3Icon::Small,20);

  setMinimumSize(300,200);
  keysListpr = new K3ListView( page );
  keysListpr->setRootIsDecorated(true);
  keysListpr->addColumn( i18n( "Name" ) );
  keysListpr->setShowSortIndicator(true);
  keysListpr->setFullWidth(true);

  labeltxt=new QLabel(i18n("Choose secret key:"),page);
  QVBoxLayout *vbox=new QVBoxLayout(page);
  vbox->setSpacing(3);

  vbox->addWidget(labeltxt);
  vbox->addWidget(keysListpr);
  if (showlocal==true)
  {
    local = new QCheckBox(i18n("Local signature (cannot be exported)"),page);
    vbox->addWidget(local);
  }

  K3ProcIO fp, fp2;
  QString tst,tst2;
  QString line;

  fp << "gpg" << "--no-tty" << "--with-colon" << "--list-secret-keys";
  fp.start();
  while ( fp.readln(line) > 0)
  {
    tst=line;
    if (tst.startsWith("sec"))
    {
      const QString trust=tst.section(':',1,1);
      QString val=tst.section(':',6,6);
      QString id=QString("0x"+tst.section(':',4,4).right(8));
      if (val.isEmpty())
        val=i18n("Unlimited");
      QString tr;
      switch( trust.at(0).toLatin1() )
      {
      case 'o':
        tr= i18n("Unknown");
        break;
      case 'i':
        tr= i18n("Invalid");
        break;
      case 'd':
        tr=i18n("Disabled");
        break;
      case 'r':
        tr=i18n("Revoked");
        break;
      case 'e':
        tr=i18n("Expired");
        break;
      case 'q':
        tr=i18n("Undefined");
        break;
      case 'n':
        tr=i18n("None");
        break;
      case 'm':
        tr=i18n("Marginal");
        break;
      case 'f':
        tr=i18n("Full");
        break;
      case 'u':
        tr=i18n("Ultimate");
        break;
      default:
        tr=i18n("?");
        break;
      }
      tst=tst.section(":",9,9);

      fp2 << "gpg" << "--no-tty" << "--with-colon" << "--list-key" << QString("%1").arg( K3ShellProcess::quote( id ) );
      fp2.start();
      bool dead=true;
      while ( fp2.readln(line) > 0)
      {
        tst2=line;
        if (tst2.startsWith("pub"))
        {
          const QString trust2=tst2.section(':',1,1);
          switch( trust2.at(0).toLatin1() )
          {
          case 'f':
            dead=false;
            break;
          case 'u':
            dead=false;
            break;
          default:
            break;
          }
        }
      }
      if (!tst.isEmpty() && (!dead))
      {
        K3ListViewItem *item=new K3ListViewItem(keysListpr,extractKeyName(tst));
        K3ListViewItem *sub= new K3ListViewItem(item,i18n("ID: %1, trust: %2, expiration: %3", id, tr, val));
        sub->setSelectable(false);
        item->setPixmap(0,keyPair);
      }
    }
  }

  QObject::connect(keysListpr,SIGNAL(doubleClicked(Q3ListViewItem *,const QPoint &,int)),this,SLOT(slotpreOk()));
  QObject::connect(keysListpr,SIGNAL(clicked(Q3ListViewItem *)),this,SLOT(slotSelect(Q3ListViewItem *)));


  connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
  keysListpr->setSelected(keysListpr->firstChild(),true);

  page->show();
  resize(this->minimumSize());
  setMainWidget(page);
}

QString KgpgSelKey::extractKeyName(QString fullName)
{
  QString kMail;
  if (fullName.indexOf("<")!=-1)
  {
    kMail=fullName.section('<',-1,-1);
    kMail.truncate(kMail.length()-1);
  }
  QString kName=fullName.section('<',0,0);
  if (kName.indexOf("(")!=-1) kName=kName.section('(',0,0);
  return QString(kMail+" ("+kName+')').trimmed();
}

void KgpgSelKey::slotpreOk()
{
  if (keysListpr->currentItem()->depth()!=0)
    return;
  else
    slotOk();
}

void KgpgSelKey::slotOk()
{
  if (keysListpr->currentItem()==NULL)
    reject();
  else
    accept();
}

void KgpgSelKey::slotSelect(Q3ListViewItem *item)
{
  if (item==NULL) return;
  if (item->depth()!=0)
  {
    keysListpr->setSelected(item->parent(),true);
    keysListpr->setCurrentItem(item->parent());
  }
}


QString KgpgSelKey::getkeyID()
{
  QString userid;
  /////  emit selected key
  if (keysListpr->currentItem()==NULL) return("");
  else
  {
    userid=keysListpr->currentItem()->firstChild()->text(0);
    userid=userid.section(',',0,0);
    userid=userid.section(':',1,1);
    userid=userid.trimmed();
    return(userid);
  }
}

QString KgpgSelKey::getkeyMail()
{
  QString username;
  /////  emit selected key
  if (keysListpr->currentItem()==NULL) return("");
  else
  {
    username=keysListpr->currentItem()->text(0);
    //username=username.section(' ',0,0);
    username=username.trimmed();
    return(username);
  }
}

bool KgpgSelKey::getlocal()
{
  /////  emit exportation choice
  return(local->isChecked());
}

#include "kgpgselkey.moc"
