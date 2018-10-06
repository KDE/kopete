/*
    ChatSessionMembersListModel

    Copyright (c) 2007 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "chatsessionmemberslistmodel.h"
#include "kopetecontact.h"
#include "kopeteonlinestatus.h"

//own contact comparator, needs sessions contact-online-status weight
inline bool lessThan(const Kopete::Contact *c1, int weight1, const Kopete::Contact *c2, int weight2)
{
    return weight1 > weight2 || (weight1 == weight2
                                 && c1->displayName().compare(c2->displayName(), Qt::CaseInsensitive) <= 0);
}

//class for more sorting speed
class ContactWrapper
{
public:
    ContactWrapper(Kopete::Contact *contact, int weight)
    {
        this->contact = contact;
        this->weight = weight;
    }

    Kopete::Contact *contact;
    int weight;
    bool operator <(const ContactWrapper &other) const
    {
        return lessThan(this->contact, weight, other.contact, other.weight);
    }
};

class Private
{
public:
    Kopete::ContactPtrList contacts;

    Kopete::ChatSession *session;

    QTimer *timer;

    //insert into temporary efficient structure for sorting, then copy
    void insertMultiple(Kopete::ContactPtrList contacts)
    {
        QMap<ContactWrapper, Kopete::Contact *> map;
        foreach (Kopete::Contact *c, this->contacts) {
            map.insert(ContactWrapper(c, session->contactOnlineStatus(c).weight()), c);
        }
        foreach (Kopete::Contact *c, contacts) {
            map.insert(ContactWrapper(c, session->contactOnlineStatus(c).weight()), c);
        }
        this->contacts.clear();
        foreach (Kopete::Contact *c, map) {
            this->contacts.append(c);
        }
    }

    //get index by binary search
    int getInsertIndex(const Kopete::Contact *contact) const
    {
        int weightOld;
        int weightNew;
        int i;
        int min = 0;
        int max = contacts.size();
        int step;

        while (max > 0)
        {
            step = max/2;
            i = min + step;

            weightOld = session->contactOnlineStatus(contacts[i]).weight();
            weightNew = session->contactOnlineStatus(contact).weight();

            if (lessThan(contacts[i], weightOld, contact, weightNew)) {
                min = ++i;
                max -= step+1;
            } else {
                max = step;
            }
        }
        return min;
    }
};

namespace Kopete {
ChatSessionMembersListModel::ChatSessionMembersListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    d = new Private();
    d->session = nullptr;
}

ChatSessionMembersListModel::~ChatSessionMembersListModel()
{
    delete d;
}

Kopete::ChatSession *ChatSessionMembersListModel::session()
{
    return d->session;
}

void ChatSessionMembersListModel::setChatSession(ChatSession *session)
{
    if (d->session) {
        disconnect(d->session, 0, this, 0);
    }

    d->session = session;

    connect(session, SIGNAL(closing(Kopete::ChatSession*)),
            this, SLOT(slotSessionClosed()));
    connect(session, SIGNAL(contactAdded(const Kopete::Contact*,bool)),
            this, SLOT(slotContactAdded(const Kopete::Contact*)));
    connect(session, SIGNAL(contactRemoved(const Kopete::Contact*,QString,Qt::TextFormat,bool)),
            this, SLOT(slotContactRemoved(const Kopete::Contact*)));
    connect(session, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
            this, SLOT(slotContactStatusChanged(Kopete::Contact*,Kopete::OnlineStatus)));
    connect(session, SIGNAL(displayNameChanged()),
            this, SLOT(slotSessionChanged()));
    connect(session, SIGNAL(photoChanged()),
            this, SLOT(slotSessionChanged()));
    connect(session, SIGNAL(nickNameChanged(Kopete::Contact*,QString)),
            this, SLOT(slotContactNickNameChanged(Kopete::Contact*)));

    d->contacts.clear();
    d->insertMultiple(d->session->members());

    d->contacts.insert(
        d->getInsertIndex((Contact *)d->session->myself()),
        (Contact *)d->session->myself());

    reset();
}

Kopete::Contact *ChatSessionMembersListModel::contactAt(const QModelIndex &index) const
{
    qCDebug(LIBKOPETE_LOG) << "memberslistmodel contactat";
    if (d->session) {
        if (!index.isValid()) {
            return nullptr;
        }

        if (index.row() >= d->contacts.size()) {
            return nullptr;
        }

        return d->contacts.at(index.row());
    }

    return nullptr;
}

int ChatSessionMembersListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (d->session) {
        return d->contacts.size();
    }

    return 0;
}

QVariant ChatSessionMembersListModel::data(const QModelIndex &index, int role) const
{
    Contact *c = d->contacts.at(index.row());
    if (!c) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return c->displayName();
    } else if (role == Qt::DecorationRole) {
        return d->session->contactOnlineStatus(c).iconFor(c);
    } else if (role == Qt::ToolTipRole) {
        return c->toolTip();
    } else {
        return QVariant();
    }
}

QVariant ChatSessionMembersListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        return QStringLiteral("Column %1").arg(section);
    } else {
        return QStringLiteral("Row %1").arg(section);
    }
}

void ChatSessionMembersListModel::slotContactAdded(const Kopete::Contact *contact)
{
    qCDebug(LIBKOPETE_LOG) << "memberslistmodel contact added "<< contact->displayName();
    int index = d->getInsertIndex(contact);
    beginInsertRows(QModelIndex(), index, index);
    d->contacts.insert(index, (Contact *)contact);
    endInsertRows();
}

void ChatSessionMembersListModel::slotContactRemoved(const Kopete::Contact *contact)
{
    qCDebug(LIBKOPETE_LOG) << "memberslistmodel contact removed "<< contact->displayName();
    int index = d->contacts.indexOf((Contact *)contact);
    if (index == -1) {
        qCDebug(LIBKOPETE_LOG) << "Trying to remove contact '" << contact->displayName() << "' which isn't in members list model!!!";
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    d->contacts.removeAt(index);
    endRemoveRows();
}

void ChatSessionMembersListModel::slotContactStatusChanged(Kopete::Contact *contact, const Kopete::OnlineStatus &status)
{
    Q_UNUSED(status)
    qCDebug(LIBKOPETE_LOG) << "memberslistmodel contact status changed "<< contact->displayName();
    slotContactRemoved(contact);
    slotContactAdded(contact);
}

void ChatSessionMembersListModel::slotSessionChanged()
{
    qCDebug(LIBKOPETE_LOG);
    reset();
}

void ChatSessionMembersListModel::slotContactNickNameChanged(Kopete::Contact *contact)
{
    qCDebug(LIBKOPETE_LOG) << "memberslistmodel nickname changed to "<< contact->displayName();
    slotContactRemoved(contact);
    slotContactAdded(contact);
}

void ChatSessionMembersListModel::slotSessionClosed()
{
    if (d->session) {
        disconnect(d->session, 0, this, 0);
        d->session = nullptr;
        reset();
    }
}
}
