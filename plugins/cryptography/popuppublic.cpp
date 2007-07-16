//File Imported from  KGPG  ( 2004 - 09 - 03 )

/***************************************************************************
                          popuppublic.cpp  -  description
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

////////////////////////////////////////////////////////   code  for choosing a public key from a list for encryption

#include <k3procio.h>
#include <KIconLoader>
#include <KActionCollection>
#include <KAction>
#include "popuppublic.h"
#include "ui_popuppublicbase.h"
#include "kgpginterface.h"

#include <cstdio>

///////////////  main view

PopupPublic::PopupPublic(QWidget *parent, const KShortcut& goDefaultKey)
 : KDialog( parent )
{
	setCaption( i18n("Select Public Key") );
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );

	ui = new Ui::PopupPublicBase();
	ui->setupUi(mainWidget());
	
        keyPair = SmallIcon("kgpg_key2");
        keySingle = SmallIcon("kgpg_key1");
	keyGroup = SmallIcon("kgpg_key3");


 	ui->listSearch->setTreeWidget(ui->keyList);

        KActionCollection *actcol=new KActionCollection(this);
	KAction *defaultKeyAction = new KAction(i18n("&Go to Default Key"), this );
        actcol->addAction( "go_default_key", defaultKeyAction );
	defaultKeyAction->setShortcut(goDefaultKey);
	connect( defaultKeyAction, SIGNAL(triggered(bool)), this, SLOT(slotGotoDefaultKey()) );

	KConfigGroup config(KGlobal::config(), "Cryptography Plugin");
	ui->untrustedCheck->setChecked(config.readEntry("UntrustedKeys", true));

        connect(ui->keyList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotOk()));
	connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
        connect(ui->untrustedCheck, SIGNAL(toggled(bool)), this, SLOT(refresh(bool)));

        char line[200]="\0";
        FILE *fp2;
        seclist.clear();

        fp2 = popen("gpg --no-secmem-warning --no-tty --with-colon --list-secret-keys ", "r");
        while ( fgets( line, sizeof(line), fp2))
        {
	QString readLine=line;
	if (readLine.startsWith("sec")) seclist+=", 0x"+readLine.section(":",4,4).right(8);
	}
        pclose(fp2);

        trusted = ui->untrustedCheck->isChecked();

        refreshkeys();
	setMinimumSize(550,200);
	updateGeometry();
	ui->keyList->setFocus();
	show();
}

PopupPublic::~PopupPublic()
{}


void PopupPublic::slotAccept()
{
accept();
}

void PopupPublic::enable()
{
	/*
	FIXME: port this code
        QListWidgetItem *current = ui->keyList->firstChild();
        if (current==NULL)
                return;
        current->setVisible(true);
        while ( current->nextSibling() ) {
                current = current->nextSibling();
                current->setVisible(true);
        }
	keysList->ensureItemVisible(keysList->currentItem());
	*/
}

void PopupPublic::sort()
{
/*
	FIXME: port this code
        bool reselect=false;
        Q3ListViewItem *current = keysList->firstChild();
        if (current==NULL)
                return;

	if ((untrustedList.indexOf(current->text(2))!=-1) && (!current->text(2).isEmpty())){
                if (current->isSelected()) {
                        current->setSelected(false);
                        reselect=true;
                }
                current->setVisible(false);
		}

        while ( current->nextSibling() ) {
                current = current->nextSibling();
                if ((untrustedList.indexOf(current->text(2))!=-1) && (!current->text(2).isEmpty())) {
                if (current->isSelected()) {
                        current->setSelected(false);
                        reselect=true;
                }
                current->setVisible(false);
		}
        }

        if (reselect) {
                Q3ListViewItem *firstvisible;
                firstvisible=keysList->firstChild();
                while (firstvisible->isVisible()!=true) {
                        firstvisible=firstvisible->nextSibling();
                        if (firstvisible==NULL)
                                return;
                }
                keysList->setSelected(firstvisible,true);
		keysList->setCurrentItem(firstvisible);
		keysList->ensureItemVisible(firstvisible);
        }
*/
}

void PopupPublic::customOpts(const QString &str)
{
        customOptions=str;
}

void PopupPublic::slotGotoDefaultKey()
{
    /*QListViewItem *myDefaulKey = keysList->findItem(KGpgSettings::defaultKey(),2);
    keysList->clearSelection();
    keysList->setCurrentItem(myDefaulKey);
    keysList->setSelected(myDefaulKey,true);
    keysList->ensureItemVisible(myDefaulKey);*/
}

void PopupPublic::refresh(bool state)
{
        if (state)
                enable();
        else
                sort();
}

void PopupPublic::refreshkeys()
{
	ui->keyList->clear();
	/*QStringList groups= QStringList::split(",", KGpgSettings::groups());
	if (!groups.isEmpty())
	{
		for ( QStringList::Iterator it = groups.begin(); it != groups.end(); ++it )
		{
			if (!QString(*it).isEmpty())
			{
				UpdateViewItem2 *item=new UpdateViewItem2(keysList,QString(*it),QString::null,QString::null,false);
				item->setPixmap(0,keyGroup);
			}
		}
	}*/
        K3ProcIO *encid=new K3ProcIO();
        *encid << "gpg"<<"--no-secmem-warning"<<"--no-tty"<<"--with-colon"<<"--list-keys";
        /////////  when process ends, update dialog infos
        QObject::connect(encid, SIGNAL(processExited(K3Process *)),this, SLOT(slotpreselect()));
        QObject::connect(encid, SIGNAL(readReady(K3ProcIO *)),this, SLOT(slotprocread(K3ProcIO *)));
        encid->start(K3Process::NotifyOnExit,true);
}

void PopupPublic::slotpreselect()
{
	/* FIXME: port this code
	QListWidgetItem *it;
        //if (fmode) it=keysList->findItem(KGpgSettings::defaultKey(),2);
        //else {
                it = ui->keyList->item(0);
                if (it==NULL)
                        return;
                while (!it->isVisible()) {
                        it=it->nextSibling();
                        if (it==NULL)
                                return;
                }
        //}
if (!trusted)
              sort();
	keysList->setSelected(it,true);
	keysList->setCurrentItem(it);
	keysList->ensureItemVisible(it);
	*/
emit keyListFilled();
}

void PopupPublic::slotSetVisible()
{
	ui->keyList->scrollToItem(ui->keyList->currentItem());
}

void PopupPublic::slotprocread(K3ProcIO *p)
{
        ///////////////////////////////////////////////////////////////// extract  encryption keys
        bool dead;
        QString tst,keyname,keymail;

        QString defaultKey ;// = KGpgSettings::defaultKey().right(8);

        while (p->readln(tst)!=-1) {
                if (tst.startsWith("pub")) {
			QStringList keyString=tst.split(":",QString::KeepEmptyParts);
                        dead=false;
                        const QString trust=keyString[1];
                        QString val=keyString[6];
                        QString id=QString("0x"+keyString[4].right(8));
                        if (val.isEmpty())
                                val=i18n("Unlimited");
                        QString tr;
                        switch( trust[0].toLatin1() ) {
                        case 'o':
				untrustedList<<id;
                                break;
                        case 'i':
                                dead=true;
                                break;
                        case 'd':
                                dead=true;
                                break;
                        case 'r':
                                dead=true;
                                break;
                        case 'e':
                                dead=true;
                                break;
                        case 'q':
                                untrustedList<<id;
                                break;
                        case 'n':
                                untrustedList<<id;
                                break;
                        case 'm':
                                untrustedList<<id;
                                break;
                        case 'f':
                                break;
                        case 'u':
                                break;
                        default:
				untrustedList<<id;
                                break;
                        }
			if (keyString[11].indexOf('D')!=-1) dead=true;
                        tst=keyString[9];
			if (tst.indexOf("<")!=-1) {
                keymail=tst.section('<',-1,-1);
                keymail.truncate(keymail.length()-1);
                keyname=tst.section('<',0,0);
                //if (keyname.find("(")!=-1)
                 //       keyname=keyname.section('(',0,0);
        } else {
                keymail.clear();
                keyname=tst;//.section('(',0,0);
        }

	keyname=GpgInterface::checkForUtf8(keyname);

                        if ((!dead) && (!tst.isEmpty())) {
				bool isDefaultKey=false;
                                if (id.right(8)==defaultKey) isDefaultKey=true;
                                QTreeWidgetItem *item=new QTreeWidgetItem(ui->keyList);
				item->setText(0, keyname);
				item->setText(1, keymail);
				item->setText(2, id);
				if (isDefaultKey)
				{
					for (int i = 0; i < 3; ++i)
					{
						QFont f = item->font(i);
						f.setBold(true);
						item->setFont(i, f);
					}
				}
                                if (seclist.indexOf(tst,0,Qt::CaseInsensitive)!=-1)
                                     item->setIcon(0,keyPair);
                                else
                                     item->setIcon(0,keySingle);
                        }
                }
        }
}


void PopupPublic::slotOk()
{
//BEGIN modified for Kopete
	KConfigGroup config(KGlobal::config(), "Cryptography Plugin");

	config.writeEntry("UntrustedKeys", ui->untrustedCheck->isChecked());

//END modified for Kopete




        //////   emit selected data
kDebug(2100)<<"Ok pressed"<<endl;
        QStringList selectedKeys;
	QString userid;
	QList<QTreeWidgetItem*> list = ui->keyList->selectedItems();

        for ( int i = 0; i < list.count(); ++i )
                if ( list.at(i) ) {
			if (!list.at(i)->text(2).isEmpty()) selectedKeys<<list.at(i)->text(2);
			else selectedKeys<<list.at(i)->text(0);
                }
        if (selectedKeys.isEmpty())
                return;
	kDebug(2100)<<"Selected Key:"<<selectedKeys<<endl;
        QStringList returnOptions;
        if (ui->untrustedCheck->isChecked())
                returnOptions<<"--always-trust";
        //MODIFIED for kopete
        emit selectedKey(selectedKeys.first(),QString(),false,false);
        accept();
}

#include "popuppublic.moc"
