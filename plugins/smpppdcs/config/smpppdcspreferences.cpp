/*
    smpppdcspreferences.cpp

    Copyright (c) 2004      by Heiko Schaefer        <heiko@rangun.de>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include <qmap.h>
#include <qregexp.h>
#include <qtabwidget.h>

#include <kconfig.h>
#include <klistview.h>
#include <kautoconfig.h>
#include <kiconloader.h>
#include <kgenericfactory.h>

#include "kopeteaccount.h"
#include "kopeteprotocol.h"
#include "kopeteaccountmanager.h"

#include "smpppdcspreferences.h"
#include "smpppdcsprefsimpl.h"

#define CONFIGGROUP "SMPPPDCS Plugin"

typedef KGenericFactory<SMPPPDCSPreferences> SMPPPDCSPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_kopete_smpppdcs, SMPPPDCSPreferencesFactory("kcm_kopete_smpppdcs"))

SMPPPDCSPreferences::SMPPPDCSPreferences(QWidget * parent, const char * /* name */, const QStringList& args)
 : KCAutoConfigModule(SMPPPDCSPreferencesFactory::instance(), parent, args), m_ui(NULL) {

 	Kopete::AccountManager * manager = Kopete::AccountManager::self(); 
	m_ui = new SMPPPDCSPrefs(this);

	for(QPtrListIterator<Kopete::Account> it(manager->accounts()); it.current(); ++it)
	{
		QString protoName;
		QRegExp rex("(.*)Protocol");
		if(rex.search((*it)->protocol()->pluginId()) > -1) {
			protoName = rex.cap(1);
		} else {
			protoName = (*it)->protocol()->pluginId();
		}
		
		QCheckListItem * cli = new QCheckListItem(m_ui->accountList, 
			(*it)->accountId() + " (" + protoName + ")", QCheckListItem::CheckBox);
		cli->setPixmap(0, (*it)->accountIcon());
		
		m_accountMapOld[cli->text(0)] = AccountPrivMap(FALSE, (*it)->protocol()->pluginId() + "_" + (*it)->accountId());
		m_accountMapCur[cli->text(0)] = AccountPrivMap(FALSE, (*it)->protocol()->pluginId() + "_" + (*it)->accountId());;
		m_ui->accountList->insertItem(cli);
	}
	
	autoConfig()->ignoreSubWidget(m_ui->tabWidget);
	autoConfig()->addWidget(m_ui->tabWidget->page(0), CONFIGGROUP);
	setMainWidget(m_ui, CONFIGGROUP);

	connect(m_ui->accountList, SIGNAL(clicked(QListViewItem *)), this, SLOT(listClicked(QListViewItem *)));
	
	load();
}

SMPPPDCSPreferences::~SMPPPDCSPreferences() {
	delete m_ui;
}

void SMPPPDCSPreferences::listClicked(QListViewItem * item)
{
	QCheckListItem * cli = dynamic_cast<QCheckListItem *>(item);
	
	if(cli->isOn() != m_accountMapCur[cli->text(0)].m_on) {
		AccountMap::iterator itOld = m_accountMapOld.begin();
		AccountMap::iterator itCur;
		bool change = FALSE;
		
		for(itCur = m_accountMapCur.begin(); itCur != m_accountMapCur.end(); ++itCur, ++itOld) {
			if((*itCur).m_on != (*itOld).m_on){
				change = TRUE;
				break;
			}
		}
		emit changed(change);
	}
	m_accountMapCur[cli->text(0)].m_on = cli->isOn();
}

void SMPPPDCSPreferences::defaults()
{
	QListViewItemIterator it(m_ui->accountList);
	while(it.current()) {
		QCheckListItem * cli = dynamic_cast<QCheckListItem *>(it.current());
		cli->setOn(FALSE);
		++it;
	}
	
	KCAutoConfigModule::defaults();
}

void SMPPPDCSPreferences::load()
{
	KConfig * config = KGlobal::config();
	config->setGroup(CONFIGGROUP);
	
	QRegExp rex("^(.*) \\((.*)\\)");
	QStringList list = config->readListEntry("ignoredAccounts");
	QListViewItemIterator it(m_ui->accountList);
	while(it.current()) {
		QCheckListItem * cli = dynamic_cast<QCheckListItem *>(it.current());
		if(rex.search(cli->text(0)) > -1) {
			bool isOn = list.contains(rex.cap(2) + "Protocol_" + rex.cap(1));
			// m_accountMapOld[cli->text(0)].m_on = isOn;
			m_accountMapCur[cli->text(0)].m_on = isOn;
			cli->setOn(isOn);
		}
		++it;
	}
	KCAutoConfigModule::load();
}

void SMPPPDCSPreferences::save()
{
	KCAutoConfigModule::save();
	KConfig * config = KGlobal::config();
	config->setGroup(CONFIGGROUP);
	
	QStringList list;
	QListViewItemIterator it(m_ui->accountList);
	while(it.current()) {
	
		QCheckListItem * cli = dynamic_cast<QCheckListItem *>(it.current());
		if(cli->isOn()) {
			list.append(m_accountMapCur[cli->text(0)].m_id);
		}
		
		++it;
	}
		
	config->writeEntry("ignoredAccounts", list);
}

#include "smpppdcspreferences.moc"
