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


#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <qdir.h>
#include <qfile.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qpainter.h>
#include <qvbox.h>
#include <qclipboard.h>
#include <qkeysequence.h>

#include <kio/netaccess.h>
#include <kurl.h>
#include <kfiledialog.h>
#include <kprocess.h>
#include <kshortcut.h>

//#include "kgpginterface.h"

#include <qpushbutton.h>
#include <qdialog.h>
#include <qwidget.h>
#include <qpopupmenu.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qvbuttongroup.h>
#include <qcombobox.h>
#include <qcheckbox.h>


#include <kurl.h>
#include <ktempfile.h>
#include <ktoolbar.h>
#include <klineedit.h>
#include <klistview.h>
#include <kdialogbase.h>
#include <kbuttonbox.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kprocio.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>

//#include "kgpg.h"
//#include "keygener.h"

//#include "popupname.h"



#include "kgpgselkey.h"
/***************////////////////////////////////////////////////////////////////////////////////////////////////////
  
////////////////   Secret key selection dialog, used when user wants to sign a key
KgpgSelKey::KgpgSelKey(QWidget *parent, const char *name,bool showlocal):KDialogBase( parent, name, true,i18n("Private Key List"),Ok | Cancel)
{
  QString keyname;
  QWidget *page = new QWidget(this);
  QLabel *labeltxt;
  KIconLoader *loader= KGlobal::iconLoader();

  keyPair=loader->loadIcon("kgpg_key2",KIcon::Small,20);

  setMinimumSize(300,100);
  keysListpr = new KListView( page );
  keysListpr->setRootIsDecorated(true);
  keysListpr->addColumn( i18n( "Name" ) );
  keysListpr->setShowSortIndicator(true);
  keysListpr->setFullWidth(true);

  labeltxt=new QLabel(i18n("Choose secret key for signing:"),page);
  QVBoxLayout *vbox=new QVBoxLayout(page,3);

  vbox->addWidget(labeltxt);
  vbox->addWidget(keysListpr);
  if (showlocal==true)
    {
      local = new QCheckBox(i18n("Local signature (cannot be exported)"),page);
      vbox->addWidget(local);
    }

  FILE *fp;
  QString tst;
  char line[130];

  fp = popen("gpg --no-tty --with-colon --list-secret-keys", "r");
  while ( fgets( line, sizeof(line), fp))
  {
      tst=line;
      if (tst.find("sec",0,FALSE)!=-1)
      {
          const QString trust=tst.section(':',1,1);
          QString val=tst.section(':',6,6);
          QString id=QString("0x"+tst.section(':',4,4).right(8));
          if (val.isEmpty())
            val=i18n("Unlimited");
          QString tr;
          switch( trust[0] )
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
          if (!tst.isEmpty())
          {
              keyname=tst.section('<',1,1);
              keyname=keyname.section('>',0,0);
              keyname+=" ("+tst.section('<',0,0)+")";
              KListViewItem *item=new KListViewItem(keysListpr,keyname);
              KListViewItem *sub= new KListViewItem(item,i18n("ID: %1, trust: %2, validity: %3").arg(id).arg(tr).arg(val));
              sub->setSelectable(false);
              item->setPixmap(0,keyPair);
          }
        }
  }
  pclose(fp);

  QObject::connect(keysListpr,SIGNAL(doubleClicked(QListViewItem *,const QPoint &,int)),this,SLOT(slotpreOk()));
  QObject::connect(keysListpr,SIGNAL(clicked(QListViewItem *)),this,SLOT(slotSelect(QListViewItem *)));


  keysListpr->setSelected(keysListpr->firstChild(),true);

  page->show();
  resize(minimumSize());
  setMainWidget(page);
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

void KgpgSelKey::slotSelect(QListViewItem *item)
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
	userid.stripWhiteSpace();
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
  username=username.section(' ',0,0);
  username=username.stripWhiteSpace();
  return(username);
}
}

bool KgpgSelKey::getlocal()
{
  /////  emit exportation choice
  return(local->isChecked());
}


#include "kgpgselkey.moc"

