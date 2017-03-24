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

#ifndef KOPETE_UI_CONTACTLISTPLAINMODEL_H
#define KOPETE_UI_CONTACTLISTPLAINMODEL_H

#include "contactlistmodel.h"

#include <kopete_export.h>

namespace Kopete {
class Group;
class Contactlist;
class MetaContact;
class ContactListElement;

namespace UI {
class MetaContactModelItem;
class GroupModelItem;
class ContactListPlainModelItem;
/**
@author Aleix Pol <aleixpol@gmail.com>
*/
class KOPETE_CONTACT_LIST_EXPORT ContactListPlainModel : public ContactListModel
{
    Q_OBJECT
public:
    ContactListPlainModel(QObject *parent = nullptr);
    ~ContactListPlainModel();

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

protected Q_SLOTS:
    void handleContactDataChange(Kopete::MetaContact *) Q_DECL_OVERRIDE;
    void appearanceConfigChanged() Q_DECL_OVERRIDE;
    void loadContactList() Q_DECL_OVERRIDE;

protected:
    void loadModelSettingsImpl(QDomElement &rootElement) Q_DECL_OVERRIDE;
    void saveModelSettingsImpl(QDomDocument &doc, QDomElement &rootElement) Q_DECL_OVERRIDE;

    bool dropMetaContacts(int row, const QModelIndex &parent, Qt::DropAction action, const QList<GroupMetaContactPair> &items) Q_DECL_OVERRIDE;

private:
    void addMetaContactImpl(Kopete::MetaContact *mc);
    QModelIndexList indexListFor(Kopete::ContactListElement *) const;

    void savePositions();
    void loadPositions();

    QHash<const Kopete::MetaContact *, int > m_addContactPosition;
    QList<Kopete::MetaContact *> m_contacts;
};
}
}

#endif
//kate: tab-width 4
