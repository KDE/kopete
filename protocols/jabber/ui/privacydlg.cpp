/*
 * privacydlg.cpp
 * Copyright (C) 2006  Remko Troncon
 * Copyright (C) 2008  Charles Connell <charles@connells.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
 
#include "privacydlg.h"
#include <QListView>
#include <KInputDialog>
#include <KMessageBox>

#include "jabberaccount.h"
#include "kopetecontact.h"
#include "privacylist.h"
#include "privacymanager.h"
#include "privacylistmodel.h"

PrivacyDlg::PrivacyDlg(JabberAccount* acc, QWidget* parent) : KDialog(parent), acc_(acc)
{
	QWidget * w = new QWidget (this);
	ui_.setupUi(w);
	setMainWidget (w);
	setAttribute(Qt::WA_DeleteOnClose);
	setCaption(i18n("%1: Privacy Lists", (acc->myself()->displayName() ) ) );
	setButtons (KDialog::Close);
	ui_.lv_rules->setFocus (Qt::PopupFocusReason);

	PrivacyManager* manager = acc->client()->privacyManager();
	connect(manager,SIGNAL(listsReceived(QString,QString,QStringList)),SLOT(updateLists(QString,QString,QStringList)));
	connect(manager,SIGNAL(listReceived(PrivacyList)),SLOT(refreshList(PrivacyList)));
	connect(manager,SIGNAL(listError()),SLOT(list_failed()));
	//connect(manager,SIGNAL(listNamesError()),SLOT(listNamesError()));
	//connect(manager,SIGNAL(listReceiveError()),SLOT(listReceiveError()));

	connect(ui_.cb_active,SIGNAL(activated(int)),SLOT(active_selected(int)));
	connect(ui_.cb_default,SIGNAL(activated(int)),SLOT(default_selected(int)));
	connect(ui_.cb_lists,SIGNAL(activated(int)),SLOT(list_selected(int)));
	connect(ui_.cb_lists,SIGNAL(currentIndexChanged(int)),SLOT(list_changed(int)));
	connect(manager,SIGNAL(changeActiveList_success()),SLOT(change_succeeded()));
	connect(manager,SIGNAL(changeActiveList_error()),SLOT(change_failed()));
	connect(manager,SIGNAL(changeDefaultList_success()),SLOT(change_succeeded()));
	connect(manager,SIGNAL(changeDefaultList_error()),SLOT(change_failed()));
	connect(manager,SIGNAL(changeList_success()),SLOT(changeList_succeeded()));
	connect(manager,SIGNAL(changeList_error()),SLOT(changeList_failed()));

	connect(ui_.pb_newList,SIGNAL(clicked()),SLOT(newList()));
	connect(ui_.pb_deleteList,SIGNAL(clicked()),SLOT(removeList()));
	
	connect(ui_.pb_add,SIGNAL(clicked()),SLOT(addRule()));
	connect(ui_.pb_edit,SIGNAL(clicked()),SLOT(editCurrentRule()));
	connect(ui_.pb_remove,SIGNAL(clicked()),SLOT(removeCurrentRule()));
	connect(ui_.pb_up,SIGNAL(clicked()),SLOT(moveCurrentRuleUp()));
	connect(ui_.pb_down,SIGNAL(clicked()),SLOT(moveCurrentRuleDown()));
	connect(ui_.pb_apply,SIGNAL(clicked()),SLOT(applyList()));
	
	ui_.pb_newList->setIcon(KIcon("list-add"));
	ui_.pb_deleteList->setIcon(KIcon("list-remove"));
	ui_.pb_add->setIcon(KIcon("list-add"));
	ui_.pb_remove->setIcon(KIcon("list-remove"));
	ui_.pb_up->setIcon(KIcon("arrow-up"));
	ui_.pb_down->setIcon(KIcon("arrow-down"));
	ui_.pb_edit->setIcon(KIcon("edit-rename"));
	ui_.pb_apply->setIcon(KIcon("dialog-ok-apply"));

	setWidgetsEnabled(false);

	// Disable all buttons
	ui_.pb_deleteList->setEnabled(false);
	setEditRuleEnabled(false);
	ui_.pb_add->setEnabled(false);
	ui_.pb_apply->setEnabled(false);

	// FIXME: Temporarily disabling auto-activate
	ui_.ck_autoActivate->hide();
	
	manager->requestListNames();
}

void PrivacyDlg::setWidgetsEnabled(bool b)
{
	ui_.gb_settings->setEnabled(b);
	ui_.gb_listSettings->setEnabled(b);
}

void PrivacyDlg::setEditRuleEnabled(bool b)
{
	ui_.pb_up->setEnabled(b);
	ui_.pb_down->setEnabled(b);
	ui_.pb_edit->setEnabled(b);
	ui_.pb_remove->setEnabled(b);
}

void PrivacyDlg::addRule()
{
	model_.add();
}

void PrivacyDlg::editCurrentRule()
{
	// Maybe should use MVC setData here
	model_.edit(ui_.lv_rules->currentIndex());
}

void PrivacyDlg::removeCurrentRule()
{
	if (ui_.lv_rules->currentIndex().isValid()) {
		model_.removeRow(ui_.lv_rules->currentIndex().row(),ui_.lv_rules->currentIndex().parent());
	}
}

void PrivacyDlg::moveCurrentRuleUp()
{
	int row = ui_.lv_rules->currentIndex().row();
	if (model_.moveUp(ui_.lv_rules->currentIndex())) {
		ui_.lv_rules->setCurrentIndex(model_.index(row-1,0));
	}
}

void PrivacyDlg::moveCurrentRuleDown()
{
	int row = ui_.lv_rules->currentIndex().row();
	if (model_.moveDown(ui_.lv_rules->currentIndex())) {
		ui_.lv_rules->setCurrentIndex(model_.index(row+1,0));
	}
}

void PrivacyDlg::applyList()
{
	if (!model_.list().isEmpty()) {
		setWidgetsEnabled(false);
		acc_->client()->privacyManager()->changeList(model_.list());
		if (newList_)
			acc_->client()->privacyManager()->requestListNames();
	}
}

void PrivacyDlg::updateLists(const QString& defaultList, const QString& activeList, const QStringList& names)
{
	// Active list
	ui_.cb_active->clear();
	ui_.cb_active->addItem(i18n("<None>"));
	ui_.cb_active->addItems(names);
	if (!activeList.isEmpty()) {
		ui_.cb_active->setCurrentIndex(names.indexOf(activeList)+1);
	}
	else {
		ui_.cb_active->setCurrentItem(0);
	}
	previousActive_ = ui_.cb_active->currentIndex();
	
	// Default list
	ui_.cb_default->clear();
	ui_.cb_default->addItem(i18n("<None>"));
	ui_.cb_default->addItems(names);
	if (!defaultList.isEmpty()) {
		ui_.cb_default->setCurrentIndex(names.indexOf(defaultList)+1);
	}
	else {
		ui_.cb_default->setCurrentItem(0);
	}
	previousDefault_ = ui_.cb_default->currentIndex();
	
	// All lists
	QString previousList = ui_.cb_lists->currentText();
	ui_.cb_lists->clear();
	ui_.cb_lists->addItems(names);
	if (ui_.cb_lists->count() > 0) {
		if (!previousList.isEmpty() && ui_.cb_lists->findText(previousList) != -1) {
			ui_.cb_lists->setCurrentIndex(ui_.cb_lists->findText(previousList));
		}
		else {
			QString currentList = (activeList.isEmpty() ? activeList : defaultList);
			if (!currentList.isEmpty()) {
				ui_.cb_lists->setCurrentIndex(names.indexOf(currentList));
			}
		}
		acc_->client()->privacyManager()->requestList(ui_.cb_lists->currentText());
	}
	else {
		setWidgetsEnabled(true);
	}
	
	ui_.lv_rules->setModel(&model_);
}

void PrivacyDlg::listChanged()
{
	if (model_.list().isEmpty()) {
		ui_.cb_lists->removeItem(previousList_);
		rememberSettings();
	}
	setWidgetsEnabled(false);
	acc_->client()->privacyManager()->requestList(ui_.cb_lists->currentText());
}

void PrivacyDlg::refreshList(const PrivacyList& list)
{
	if (list.name() == ui_.cb_lists->currentText()) {
		rememberSettings();
		model_.setList(list);
		setWidgetsEnabled(true);
	}
}

void PrivacyDlg::active_selected(int i)
{
	if (i != previousActive_) {
		setWidgetsEnabled(false);
		acc_->client()->privacyManager()->changeActiveList((i == 0 ? "" : ui_.cb_active->itemText(i)));
	}
}

void PrivacyDlg::default_selected(int i)
{
	if (i != previousDefault_) {
		setWidgetsEnabled(false);
		acc_->client()->privacyManager()->changeDefaultList((i == 0 ? "" : ui_.cb_active->itemText(i)));
	}
}

void PrivacyDlg::list_selected(int i)
{
	if (i != previousList_) {
		listChanged();
	}
}

void PrivacyDlg::list_changed(int i)
{
	ui_.pb_deleteList->setEnabled(i != -1);
	ui_.pb_add->setEnabled(i != -1);
	setEditRuleEnabled(i != -1);
	ui_.pb_apply->setEnabled(i != -1);
	//setEditRuleEnabled(false);
	newList_ = false;
}

void PrivacyDlg::list_failed()
{
	revertSettings();
	setWidgetsEnabled(true);
}

void PrivacyDlg::changeList_succeeded()
{
	// If we just deleted a list, select the first list
	if (model_.list().isEmpty()) {
		ui_.cb_lists->setCurrentIndex(0);
		listChanged();
	}
	else {
		setWidgetsEnabled(true);
	}
}

void PrivacyDlg::changeList_failed()
{
	KMessageBox::error(0, i18n("There was an error changing the list."), i18n("Error")); 
	setWidgetsEnabled(true);
}

void PrivacyDlg::change_succeeded()
{
	rememberSettings();
	setWidgetsEnabled(true);
}

void PrivacyDlg::change_failed()
{
	revertSettings();
	KMessageBox::error(0, i18n("There was an error processing your request."), i18n("Error")); 
	setWidgetsEnabled(true);
}

void PrivacyDlg::rememberSettings()
{
	previousDefault_ = ui_.cb_default->currentIndex();
	previousActive_ = ui_.cb_active->currentIndex();
	previousList_ = ui_.cb_lists->currentIndex();
}

void PrivacyDlg::revertSettings()
{
	ui_.cb_default->setCurrentIndex(previousDefault_);
	ui_.cb_active->setCurrentIndex(previousActive_);
	ui_.cb_lists->setCurrentIndex(previousList_);
}


void PrivacyDlg::newList()
{
	bool done = false;
	bool ok = false;
	QString name;
	while (!done) {
		name = KInputDialog::getText(i18n("New List"), i18n("Enter the name of the new list:"), QString(), &ok, this);
		if (!ok) {
			done = true;
		}
		else if (ui_.cb_lists->findText(name) != -1) {
			KMessageBox::error(this, i18n("A list with this name already exists."), i18n("Error"));
		}
		else if (!name.isEmpty()) {
			done = true;
		}
	}
	
	if (ok) {
		if (ui_.cb_lists->currentIndex() != -1 && model_.list().isEmpty()) {
			ui_.cb_lists->removeItem(ui_.cb_lists->currentIndex());
		}
		ui_.cb_lists->addItem(name);
		ui_.cb_lists->setCurrentIndex(ui_.cb_lists->findText(name));
		model_.setList(PrivacyList(name));
		newList_ = true;
		rememberSettings();
	}
}

void PrivacyDlg::removeList()
{
	model_.list().clear();
	acc_->client()->privacyManager()->changeList(model_.list());
	acc_->client()->privacyManager()->requestListNames();
}
