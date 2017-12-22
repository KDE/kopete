/*
 * privacydlg.h
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
 
#ifndef PRIVACYDLG_H
#define PRIVACYDLG_H

#include <KDialog>

#include "ui_privacy.h"
#include "privacylistmodel.h"

class JabberAccount;
class QWidget;
class QString;
class QStringList;

class PrivacyDlg : public KDialog
{
	Q_OBJECT

public:
	PrivacyDlg(JabberAccount* acc, QWidget* parent);
	~PrivacyDlg() { };

protected:
	void rememberSettings();
	void revertSettings();
	void listChanged();
	
protected Q_SLOTS:
	void setWidgetsEnabled(bool);
	void setEditRuleEnabled(bool);
	void updateLists(const QString&, const QString&, const QStringList&);
	void refreshList(const PrivacyList&);
	void active_selected(int);
	void default_selected(int);
	void list_selected(int i);
	void list_changed(int);
	void list_failed();
	void changeList_succeeded();
	void changeList_failed();
	void change_succeeded();
	void change_failed();
	void addRule();
	void editCurrentRule();
	void removeCurrentRule();
	void moveCurrentRuleUp();
	void moveCurrentRuleDown();
	void applyList();
	void newList();
	void removeList();

private:
	Ui::Privacy ui_;
	int previousActive_, previousDefault_, previousList_;
	JabberAccount* acc_;
	PrivacyListModel model_;
	bool newList_;
};

#endif
