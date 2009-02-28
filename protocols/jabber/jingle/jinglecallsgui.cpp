 /*
  * jinglecallsgui.cpp - A GUI which displays all Jingle calls.
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
#include "jinglecallsgui.h"
#include "jinglecallsmanager.h"
#include "jabberjinglesession.h"
#include "jabberjinglecontent.h"

#include "jinglesession.h"
#include "jinglecontent.h"

#include <KDebug>

static QString stateToString(JabberJingleSession::State s)
{
	//TODO : more precise states...
	switch (s)
	{
	case JabberJingleSession::Pending :
		return "Pending";
	case JabberJingleSession::Active :
		return "Active";
	case JabberJingleSession::Ended :
		return "Ended";
	default :
		return "Unknown";
	}
}

JingleCallsGui::JingleCallsGui(JingleCallsManager* parent)
: m_callsManager(parent)
{
	kDebug() << "Created";
	ui.setupUi(this);
	setWindowTitle("Jingle calls");
	setupActions();

	model = new JingleCallsModel(m_callsManager->jabberSessions());
	ui.treeView->setModel(model);

	updater = new QTimer();
	connect(updater, SIGNAL(timeout()), this, SLOT(updateTime()));
	updater->start(1000);
}

JingleCallsGui::~JingleCallsGui()
{
	kDebug() << "deleted";
	delete updater;
	delete model;
}

void JingleCallsGui::setupActions()
{
	// Create a new session
	QAction *newSession = new QAction(tr("Add Content"), this);
	ui.toolBar->addAction(newSession);
	connect(newSession, SIGNAL(triggered()), this, SLOT(slotAddContent()));

	// Adds a content to the session.
	QAction *addContent = new QAction(tr("New Session"), this);
	ui.toolBar->addAction(addContent);
	connect(addContent, SIGNAL(triggered()), this, SLOT(slotNewSession()));

	// Terminate the whole session.
	QAction *terminate = new QAction(tr("Terminate"), this);
	ui.toolBar->addAction(terminate);
	connect(terminate, SIGNAL(triggered()), this, SLOT(slotTerminate()));

	// ^ Actions on sessions
	ui.toolBar->addSeparator();
	// v Actions on contents
	
	// Remove the content.
	QAction *remove = new QAction(tr("Remove"), this);
	ui.toolBar->addAction(remove);
	connect(remove, SIGNAL(triggered()), this, SLOT(slotRemove()));

	ui.toolBar->addSeparator();

	// quit
	QAction *close = new QAction(tr("Close"), this);
	ui.toolBar->addAction(close);
	connect(close, SIGNAL(triggered()), this, SLOT(slotClose()));
}

void JingleCallsGui::slotNewSession()
{
	//TODO:Implement me !
}

void JingleCallsGui::slotAddContent()
{
	//TODO:Implement me !
}

void JingleCallsGui::slotTerminate()
{
	kDebug() << "Terminate session";
	TreeItem *item = static_cast<TreeItem*>(ui.treeView->currentIndex().internalPointer());
	if (item == 0 || item->session() == 0)
		return;
	item->session()->jingleSession()->sessionTerminate(); //FIXME:Maybe ask the manager to do that...
	removeSession(item->session());
	//TODO:Implement me !
	
}

void JingleCallsGui::slotRemove()
{
	//TODO:Implement me !
}

void JingleCallsGui::slotClose()
{
	hide();
}

void JingleCallsGui::addSession(JabberJingleSession* sess)
{
	kDebug() << "Add session" << sess;
	if (!sess)
		return;
	
	// Insert Session
	QAbstractItemModel *model = ui.treeView->model();
	QModelIndex index = model->index(model->rowCount() - 1, 0);

	if (!model->insertRow(model->rowCount(), index.parent()))
		return;

	//kDebug() << "Session inserted in the model !";


	QVector<QVariant> sessData;
	sessData << sess->session()->to().full();
	sessData << stateToString(sess->state());
	sessData << sess->upTime().toString("HH:mm"); // FIXME: find a better formatting : don't show 0 at the beginning (no 00:03)

	for (int column = 0; column < model->columnCount(index.parent()); ++column)
	{
		QModelIndex child = model->index(index.row() + 1, column, index.parent());
		TreeItem *item = static_cast<TreeItem*>(child.internalPointer());
		item->setSessionPtr(sess);
		model->setData(child, sessData[column], Qt::EditRole);
	}

	// Insert Contents
	index = model->index(model->rowCount() - 1, 0);
	
	if (!model->insertRows(model->rowCount(), sess->contents().count(), index))
		return;

	//kDebug() << "Contents inserted in the model !";

	for (int i = 0; i < sess->contents().count(); i++)
	{
		QVector<QVariant> contData;
		contData << sess->contents()[i]->contentName();
		contData << "";
		contData << "";
		
		for (int column = 0; column < model->columnCount(index.parent()); ++column)
		{
			QModelIndex child = model->index(i, column, index);
			model->setData(child, contData[column], Qt::EditRole);
		}
	}
}

void JingleCallsGui::changeState(JabberJingleSession *sess)
{
	JabberJingleSession::State s = sess->state();
	
	int i = 0;

	//Looking for the QModelIndex with the session sess.
	//root QModelIndex
	QAbstractItemModel *model = ui.treeView->model();
	QModelIndex child = model->index(0, 0);
	//QModelIndex child = rootIndex.child(0, 0);

	while (child.isValid())
	{
		kDebug() << child.data();
		TreeItem *childItem = static_cast<TreeItem*>(child.internalPointer());
		if (childItem == 0)
			kDebug() << "childItem is NULL";
		if (childItem->session() == sess)
		{
			// We have the right index :)
			//model->removeRow(i, child.parent());
			model->setData(model->index(child.row(), 1), QVariant(stateToString(s)), Qt::DisplayRole);
			break;
		}
		child = model->index(++i, 0);
	}
}

//This is run once every second.
void JingleCallsGui::updateTime()
{
	int i = 0;

	QAbstractItemModel *model = ui.treeView->model();
	QModelIndex child = model->index(0, 0);

	while (child.isValid())
	{
		TreeItem *childItem = static_cast<TreeItem*>(child.internalPointer());
		
		if (childItem->session() != 0)
		{
			// We have a session index, let's update it :)
			QTime t = childItem->session()->upTime();
			model->setData(model->index(child.row(), 2), QVariant(t.toString()), Qt::DisplayRole);
		}
		child = model->index(++i, 0);
	}
}

void JingleCallsGui::removeSession(JabberJingleSession* sess)
{
	kDebug() << "Remove session" << sess;
	int i = 0;

	//Looking for the QModelIndex with the session sess.
	//root QModelIndex
	QAbstractItemModel *model = ui.treeView->model();
	QModelIndex child = model->index(0, 0);
	//QModelIndex child = rootIndex.child(0, 0);

	while (child.isValid())
	{
		kDebug() << child.data();
		TreeItem *childItem = static_cast<TreeItem*>(child.internalPointer());
		if (childItem == 0)
			kDebug() << "childItem is NULL";
		kDebug() << "Compare" << childItem->session() << "to" << sess;
		if (childItem->session() == sess)
		{
			//Children of this line will be automatically removed.
			model->removeRow(i, child.parent());
		}
		child = model->index(++i, 0);
	}

	//Don't delete it, just remove it from the QTreeView.
}

/*DEPRECATED(sessions retrieved from m_callsManager)*/void JingleCallsGui::setSessions(const QList<JabberJingleSession*>& sessions)
{
	Q_UNUSED(sessions)

}

// Model/View

// JingleCallsModel :

JingleCallsModel::JingleCallsModel(const QList<JabberJingleSession*> &data, QObject *parent)
 : QAbstractItemModel(parent)
{
	kDebug() << "Create Model";
	QVector<QVariant> rootData;
	rootData << "Session with" << "State" << "Time";
	rootItem = new TreeItem(rootData);
	setModelUp(data);
}

void JingleCallsModel::setModelUp(const QList<JabberJingleSession*> &sessions)
{
	for (int i = 0; i < sessions.count(); i++)
	{
		QVector<QVariant> sessData;
		sessData << sessions[i]->session()->to().full();
		sessData << stateToString(sessions[i]->state());
		sessData << sessions[i]->upTime().toString("HH:mm"); // FIXME: find a better formatting : don't show 0 at the beginning (no 00:03)
		TreeItem *sessItem = new TreeItem(sessData, rootItem);
		sessItem->setSessionPtr(sessions[i]);
		for (int j = 0; j < sessions[i]->contents().count(); j++)
		{
			QVector<QVariant> contData;
			contData << sessions[i]->contents()[j]->contentName();
			TreeItem *contItem = new TreeItem(contData, sessItem);
			contItem->setContentPtr(sessions[i]->contents()[j]);
			sessItem->appendChild(contItem);
		}
		rootItem->appendChild(sessItem);
	}
}

JingleCallsModel::~JingleCallsModel()
{
	delete rootItem;
}

QModelIndex JingleCallsModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	TreeItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	TreeItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex JingleCallsModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
	TreeItem *parentItem = childItem->parent();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int JingleCallsModel::rowCount(const QModelIndex& parent) const
{
	TreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}

int JingleCallsModel::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return rootItem->columnCount();
}

QVariant JingleCallsModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if ((role != Qt::DisplayRole) && (role != Qt::EditRole))
		return QVariant();

	TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

	return item->data(index.column());
}

QVariant JingleCallsModel::headerData(int section, Qt::Orientation orientation,
			       int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}

Qt::ItemFlags JingleCallsModel::flags(const QModelIndex& index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;
	
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool JingleCallsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (role != Qt::EditRole && role != Qt::DisplayRole)
		return false;

	TreeItem *item = getItem(index);

	bool ret = item->setData(index.column(), value);
	if (ret)
		emit dataChanged(this->index(0, 0), this->index(rootItem->childCount(), 3));
		//FIXME: decrease the number of items in the interval.
	
	return ret;
}

bool JingleCallsModel::insertRows(int position, int rows, const QModelIndex& parent)
{
	TreeItem *parentItem = getItem(parent);
	bool success = true;

	beginInsertRows(parent, position, position + rows - 1);
	for (int i = 0; i < rows; i++)
	{
		if (!parentItem->appendChild(rootItem->columnCount()))
		{
			success = false;
			break;
		}
	}
	endInsertRows();

	return success;
}

void JingleCallsModel::printTree()
{
	kDebug() << "|-(rootItem)" << rootItem->data(0) << "|" << rootItem->data(1) << "|" << rootItem->data(2);
	for (int i = 0; i < rootItem->childCount(); i++)
	{
		TreeItem *sessItem = rootItem->child(i);
		kDebug() << " |-" << sessItem->data(0) << "|" << sessItem->data(1) << "|" << sessItem->data(2);
		for (int j = 0; j < sessItem->childCount(); j++)
		{
			TreeItem *contItem = sessItem->child(j);
			kDebug() << "  |-" << contItem->data(0);
		}

	}
}

bool JingleCallsModel::removeRows(int position, int rows, const QModelIndex& parent)
{
	TreeItem *parentItem = getItem(parent);
	bool success = true;

	beginRemoveRows(parent, position, position + rows - 1);
	success = parentItem->removeChildren(position, rows);
	endRemoveRows();

	return success;
}

TreeItem *JingleCallsModel::getItem(const QModelIndex& index) const
{
	if (index.isValid())
	{
		TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
		if (item)
			return item;
	}
	//kDebug() << "Return ROOT Item";
	return rootItem;
}


// TreeItem

TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent)
{
	parentItem = parent;
	itemData = data;
	contentPtr = 0L;
	sessionPtr = 0L;
}

TreeItem::~TreeItem()
{
	for (int i = 0; i < childItems.count(); i++)
	{
		delete childItems.takeAt(i);
	}
}

void TreeItem::setContentPtr(JabberJingleContent* c)
{
	contentPtr = c;
}

void TreeItem::setSessionPtr(JabberJingleSession* s)
{
	sessionPtr = s;
}

void TreeItem::appendChild(TreeItem* child)
{
	childItems.append(child);
}

TreeItem *TreeItem::child(int row)
{
	return childItems.value(row);
}

bool TreeItem::removeChildren(int pos, int count)
{
	if (pos < 0 || pos + count > childItems.size())
		return false;

	for (int row = 0; row < count; ++row)
	{
		delete childItems.takeAt(pos);
	}

	return true;
}

bool TreeItem::appendChild(int columns)
{
	//kDebug() << "Adding a TreeItem with" << columns << "columns";
	QVector<QVariant> data(columns);
	TreeItem *item = new TreeItem(data, this);
	childItems.append(item);

	return true;	
}

bool TreeItem::setData(int column, const QVariant &val)
{
	if (column < 0 || column >= itemData.size())
		return false;

	//kDebug() << "Set value" << val << "in column" << column;
	itemData[column] = val;
	return true;
}

int TreeItem::childCount() const
{
	return childItems.count();
}

int TreeItem::row() const
{
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

	return 0;
}

int TreeItem::columnCount() const
{
	return itemData.count();
}

QVariant TreeItem::data(int column) const
{
	return itemData.value(column);
}

TreeItem *TreeItem::parent()
{
	return parentItem;
}
