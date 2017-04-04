/*
    Kopete Contactlist Model

    Copyright (c) 2007      by Aleix Pol              <aleixpol@gmail.com>
    Copyright     2009      by Roman Jarosz           <kedgedev@gmail.com>

    Kopete    (c) 2002-2009 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETE_UI_CONTACTLISTTREEMODEL_H
#define KOPETE_UI_CONTACTLISTTREEMODEL_H

#include "contactlistmodel.h"

#include <kopetecontactlist_export.h>

namespace Kopete {
class Group;
class MetaContact;
class ContactListElement;

namespace UI {
class MetaContactModelItem;
class GroupModelItem;
class ContactListModelItem;
/**
@author Aleix Pol <aleixpol@gmail.com>
*/
class KOPETECONTACTLIST_EXPORT ContactListTreeModel : public ContactListModel
{
    Q_OBJECT
public:
    ContactListTreeModel(QObject *parent = nullptr);
    ~ContactListTreeModel();

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) Q_DECL_OVERRIDE;

public Q_SLOTS:
    void addMetaContact(Kopete::MetaContact *) Q_DECL_OVERRIDE;
    void removeMetaContact(Kopete::MetaContact *) Q_DECL_OVERRIDE;

    void addGroup(Kopete::Group *) Q_DECL_OVERRIDE;
    void removeGroup(Kopete::Group *) Q_DECL_OVERRIDE;

    void addMetaContactToGroup(Kopete::MetaContact *, Kopete::Group *) Q_DECL_OVERRIDE;
    void removeMetaContactFromGroup(Kopete::MetaContact *, Kopete::Group *) Q_DECL_OVERRIDE;

protected Q_SLOTS:
    void handleContactDataChange(Kopete::MetaContact *) Q_DECL_OVERRIDE;
    void appearanceConfigChanged() Q_DECL_OVERRIDE;
    void loadContactList() Q_DECL_OVERRIDE;

protected:
    void loadModelSettingsImpl(QDomElement &rootElement) Q_DECL_OVERRIDE;
    void saveModelSettingsImpl(QDomDocument &doc, QDomElement &rootElement) Q_DECL_OVERRIDE;

    bool dropMetaContacts(int row, const QModelIndex &parent, Qt::DropAction action, const QList<GroupMetaContactPair> &items) Q_DECL_OVERRIDE;

private:
    ContactListModelItem *itemFor(const QModelIndex &index) const;
    QModelIndex indexFor(ContactListModelItem *modelItem) const;
    QModelIndexList indexListFor(Kopete::ContactListElement *) const;

    int countConnected(GroupModelItem *gmi) const;
    QVariant metaContactImage(Kopete::MetaContact *mc) const;
    QString metaContactTooltip(Kopete::MetaContact *metaContact) const;

    QHash<GroupMetaContactPair, int> m_addContactPosition;

    GroupModelItem *m_topLevelGroup;
    QHash<Kopete::Group *, GroupModelItem *> m_groups;
    QMap<GroupMetaContactPair, MetaContactModelItem *> m_metaContacts;
};
}
}

#endif
//kate: tab-width 4
