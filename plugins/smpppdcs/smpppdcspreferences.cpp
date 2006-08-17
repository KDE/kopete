/*
    smpppdcspreferences.cpp

    Copyright (c) 2004-2006 by Heiko Schaefer        <heiko@rangun.de>

    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include <qlayout.h>
#include <qregexp.h>
#include <qradiobutton.h>

#include <klistview.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kgenericfactory.h>

#include "kopeteaccount.h"
#include "kopeteprotocol.h"
#include "kopeteaccountmanager.h"

#include "smpppdlocationwidget.h"
#include "smpppdcspreferences.h"
#include "smpppdcsprefsimpl.h"
#include "smpppdcsconfig.h"

typedef KGenericFactory<SMPPPDCSPreferences> SMPPPDCSPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_kopete_smpppdcs, SMPPPDCSPreferencesFactory("kcm_kopete_smpppdcs"))

SMPPPDCSPreferences::SMPPPDCSPreferences(QWidget * parent, const char * /* name */, const QStringList& args)
 : KCModule(SMPPPDCSPreferencesFactory::instance(), parent, args), m_ui(NULL) {

 	Kopete::AccountManager * manager = Kopete::AccountManager::self(); 
	(new QVBoxLayout(this))->setAutoAdd(true);
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
		
		if(it.current()->inherits("Kopete::ManagedConnectionAccount")) {
			protoName += QString(", %1").arg(i18n("connection status is managed by Kopete"));
		}
		
		QCheckListItem * cli = new QCheckListItem(m_ui->accountList, 
				(*it)->accountId() + " (" + protoName + ")", QCheckListItem::CheckBox);
		cli->setPixmap(0, (*it)->accountIcon());
		
		m_accountMapOld[cli->text(0)] = AccountPrivMap(FALSE, (*it)->protocol()->pluginId() + "_" + (*it)->accountId());
		m_accountMapCur[cli->text(0)] = AccountPrivMap(FALSE, (*it)->protocol()->pluginId() + "_" + (*it)->accountId());;
		m_ui->accountList->insertItem(cli);
	}

	connect(m_ui->accountList, SIGNAL(clicked(QListViewItem *)), this, SLOT(listClicked(QListViewItem *)));
	
	// connect for modified
	connect(m_ui->useNetstat, SIGNAL(clicked()), this, SLOT(slotModified()));
	connect(m_ui->useSmpppd,  SIGNAL(clicked()), this, SLOT(slotModified()));
	
	connect(m_ui->SMPPPDLocation->server,   SIGNAL(textChanged(const QString&)), this, SLOT(slotModified()));
	connect(m_ui->SMPPPDLocation->port,     SIGNAL(valueChanged(int)), this, SLOT(slotModified()));
	connect(m_ui->SMPPPDLocation->Password, SIGNAL(textChanged(const QString&)), this, SLOT(slotModified()));
	
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
		emit KCModule::changed(change);
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
	
	SMPPPDCSConfig::self()->setDefaults();
	
	m_ui->useNetstat->setChecked(SMPPPDCSConfig::self()->useNetstat());
	m_ui->useSmpppd->setChecked(SMPPPDCSConfig::self()->useSmpppd());
	
	m_ui->SMPPPDLocation->server->setText(SMPPPDCSConfig::self()->server());
	m_ui->SMPPPDLocation->port->setValue(SMPPPDCSConfig::self()->port());
	m_ui->SMPPPDLocation->Password->setText(SMPPPDCSConfig::self()->password());
}

void SMPPPDCSPreferences::load()
{
	
	SMPPPDCSConfig::self()->readConfig();
	
	static QString rexStr = "^(.*) \\((.*)\\)";
	QRegExp rex(rexStr);
	QStringList list = SMPPPDCSConfig::self()->ignoredAccounts();
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
	
	m_ui->useNetstat->setChecked(SMPPPDCSConfig::self()->useNetstat());
	m_ui->useSmpppd->setChecked(SMPPPDCSConfig::self()->useSmpppd());
	
	m_ui->SMPPPDLocation->server->setText(SMPPPDCSConfig::self()->server());
	m_ui->SMPPPDLocation->port->setValue(SMPPPDCSConfig::self()->port());
	m_ui->SMPPPDLocation->Password->setText(SMPPPDCSConfig::self()->password());
	
	emit KCModule::changed(false);
}

void SMPPPDCSPreferences::save()
{
	QStringList list;
	QListViewItemIterator it(m_ui->accountList);
	while(it.current()) {
	
		QCheckListItem * cli = dynamic_cast<QCheckListItem *>(it.current());
		if(cli->isOn()) {
			list.append(m_accountMapCur[cli->text(0)].m_id);
		}
		
		++it;
	}
	
	SMPPPDCSConfig::self()->setIgnoredAccounts(list);
	
	SMPPPDCSConfig::self()->setUseNetstat(m_ui->useNetstat->isChecked());
	SMPPPDCSConfig::self()->setUseSmpppd(m_ui->useSmpppd->isChecked());
	
	SMPPPDCSConfig::self()->setServer(m_ui->SMPPPDLocation->server->text());
	SMPPPDCSConfig::self()->setPort(m_ui->SMPPPDLocation->port->value());
	SMPPPDCSConfig::self()->setPassword(m_ui->SMPPPDLocation->Password->text());
	
	SMPPPDCSConfig::self()->writeConfig();
	
	emit KCModule::changed(false);
}

void SMPPPDCSPreferences::slotModified() {
	emit KCModule::changed(true);
}

#include "smpppdcspreferences.moc"
