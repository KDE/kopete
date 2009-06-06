 /*
  * jinglecallsgui.h - A GUI which displays all Jingle calls.
  *
  * Copyright (c) 2008 by Detlev Casanova <detlev.casanova@gmail.com>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */
#ifndef JINGLE_CALLS_GUI_H
#define JINGLE_CALLS_GUI_H

#include <QAbstractItemModel>

#include "ui_jinglecallsgui.h"

class JingleCallsManager;
class JabberJingleSession;
class JabberJingleContent;
class JingleCallsModel;
class JingleCallsGui : public QMainWindow
{
	Q_OBJECT
public:
	JingleCallsGui(JingleCallsManager*);
	~JingleCallsGui();

	void addSession(JabberJingleSession*);
	void setSessions(const QList<JabberJingleSession*>&);
	void removeSession(JabberJingleSession*);
	void changeState(JabberJingleSession*);

public slots:
	void slotNewSession();
	void slotAddContent();
	void slotTerminate();
	void slotRemove();
	void slotClose();
	void updateTime();

private:
	void setupActions();
	JingleCallsManager *m_callsManager;
	Ui::jingleCallsGui ui;
	JingleCallsModel *model;
	QTimer *updater;
};


class TreeItem;
class JingleCallsModel : public QAbstractItemModel
{
public:
	JingleCallsModel(const QList<JabberJingleSession*>&, QObject* parent = 0);
	~JingleCallsModel();

	QModelIndex index(int, int, const QModelIndex& parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex&) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const;
	QVariant headerData(int, Qt::Orientation, int) const;

	Qt::ItemFlags flags(const QModelIndex& index) const;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex());
	bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex());
	void printTree();

private:
	TreeItem *rootItem;
	TreeItem *getItem(const QModelIndex& index) const;
	void setModelUp(const QList<JabberJingleSession*>&);
};

/*
 * TreeItem's only contain data about what they represent (session or content)
 * TODO:add a pointer ti the session or content to retrieve it more easily. (Not sure it could work that way.)
 */
class TreeItem
{
public:
	TreeItem(const QVector<QVariant>& data, TreeItem *parent = 0);
	~TreeItem();

	void appendChild(TreeItem *child);
	TreeItem *child(int row);
	int childCount() const;
	int columnCount() const;
	QVariant data(int column) const;
	
	//bool insertChildren(int pos, int count, int columns);
	bool appendChild(int columns);
	bool removeChildren(int pos, int count);
	bool setData(int column, const QVariant& val);

	int row() const;
	TreeItem *parent();
	void setContentPtr(JabberJingleContent*);
	void setSessionPtr(JabberJingleSession*);
	JabberJingleContent* content() const {return contentPtr;}
	JabberJingleSession* session() const {return sessionPtr;}

private:
	QList<TreeItem*> childItems;
	QVector<QVariant> itemData;
	TreeItem *parentItem;
	JabberJingleContent *contentPtr;
	JabberJingleSession *sessionPtr;

};

#endif
