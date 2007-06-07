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

#include "selectkeydialog.h"

#include <cstdio>

SelectKeyDialog::SelectKeyDialog(QWidget *parent, bool showlocal)
 : KDialog( parent )
{
  setCaption( i18n("Private Key List") );
  setButtons( KDialog::Ok | KDialog::Cancel );

  QWidget *page = new QWidget(this);
  QLabel *labeltxt;

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

  fp = new K3ProcIO;
  *fp << "gpg" << "--no-tty" << "--with-colon" << "--list-secret-keys";
  connect (fp, SIGNAL(readReady(K3ProcIO*)), this, SLOT(slotReadKey(K3ProcIO*)));
  fp->start();

  QObject::connect(keysListpr,SIGNAL(doubleClicked(Q3ListViewItem *,const QPoint &,int)),this,SLOT(slotpreOk()));
  QObject::connect(keysListpr,SIGNAL(clicked(Q3ListViewItem *)),this,SLOT(slotSelect(Q3ListViewItem *)));

  keysListpr->setSelected(keysListpr->firstChild(),true);

  page->show();
  resize(this->minimumSize());
  setMainWidget(page);
}

SelectKeyDialog::~SelectKeyDialog ()
{
  delete fp;
}

void SelectKeyDialog::slotReadKey (K3ProcIO * pio)
{
  QString line, expr, id;
  pio->readln(line, false);
  if (line.startsWith("sec"))
  {
    QString expr = line.section(':',6,6);
    if (expr.isEmpty())
      expr = i18nc ("adj", "Unlimited");
    QString id = QString("0x"+line.section(':',4,4).right(8));
    K3ListViewItem *item = new K3ListViewItem (keysListpr, line.section (':', 9, 9) );
    K3ListViewItem *sub = new K3ListViewItem (item,i18n("ID: %1, expiration: %2", id, expr));
    sub->setSelectable(false);
    KIconLoader *loader = KIconLoader::global();
    keyPair=loader->loadIcon("kgpg_key2",K3Icon::Small,20);
    item->setPixmap(0,keyPair);
  }
  pio->ackRead();
}

void SelectKeyDialog::slotpreOk()
{
  if (keysListpr->currentItem()->depth()!=0)
    return;
  else
    slotOk();
}

void SelectKeyDialog::slotOk()
{
  if (keysListpr->currentItem()==NULL)
    reject();
  else
    accept();
}

void SelectKeyDialog::slotSelect(Q3ListViewItem *item)
{
  if (item==NULL) return;
  if (item->depth()!=0)
  {
    keysListpr->setSelected(item->parent(),true);
    keysListpr->setCurrentItem(item->parent());
  }
}


QString SelectKeyDialog::getkeyID()
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

QString SelectKeyDialog::getkeyMail()
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

bool SelectKeyDialog::getlocal()
{
  /////  emit exportation choice
  return(local->isChecked());
}

#include "selectkeydialog.moc"
