
/***************************************************************************
                          dlgjabberservices.h  -  description
                             -------------------
    begin                : Mon Dec 9 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGJABBERSERVICES_H
#define DLGJABBERSERVICES_H

#include <qwidget.h>

#include "jabberaccount.h"
#include "xmpp_tasks.h"

#include "ui_dlgservices.h"

/**
  *@author Till Gerken <till@tantalo.net>
  */

class ServiceItem;

class dlgJabberServices : public KDialog
{
	Q_OBJECT
public:
	  explicit dlgJabberServices(JabberAccount *account, QWidget *parent = 0);
	 ~dlgJabberServices ();

protected:
	void initTree();
	bool eventFilter(QObject *object, QEvent *event);

private slots:
	//void slotSetSelection (Q3ListViewItem *);
	void slotItemExpanded(QTreeWidgetItem *item);
	void slotService();
	void slotServiceFinished();
	void slotDisco();
	void slotDiscoFinished();
	void slotRegister();
	void slotSearch();
	void slotCommand();

private:
	Ui::dlgServices ui;
	JabberAccount  *mAccount;
	ServiceItem    *mRootItem;
	QAction        *mActRegister;
	QAction        *mActSearch;
	QAction        *mActCommand;
};

class ServiceItem : protected QObject, public QTreeWidgetItem
{
	Q_OBJECT
public:
	ServiceItem(JabberAccount *account, const QString &jid , const QString &node, const QString &name);

	const QString jid() const {return mJid; }
	const QString node() const {return mNode; }
	const XMPP::Features &features() const {return mFeatures; }

	void startDisco();

private slots:
	void slotDiscoFinished();
	void slotDiscoInfoFinished();

private:
	JabberAccount *mAccount;
	bool    mDiscoReady;
	QString mJid;
	QString mNode;
	XMPP::Features mFeatures;
};

#endif
