/*
 * connectionmanageritem.h - UI to edit Telepathy account settings
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 *               2009 by Dariusz Mikulski <dariusz.mikulski@gmail.com>
 *
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */

#ifndef _CONNECTIONMANAGERITEM_H
#define	_CONNECTIONMANAGERITEM_H

#include <QObject>
#include <QPointer>
#include <QTreeWidgetItem>

class QTreeWidget;
namespace Telepathy
{
    namespace Client
    {
        class ConnectionManager;
        class PendingOperation;
    }
}

class ConnectionManagerItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT
public:
    ConnectionManagerItem(QString cmName, QTreeWidget *parent);

    Telepathy::Client::ConnectionManager *connectionManager();

private slots:
    void setProtocolsSize(Telepathy::Client::PendingOperation *p);

private:
    QPointer<Telepathy::Client::ConnectionManager> m_connectionManager;
};

#endif	/* _CONNECTIONMANAGERITEM_H */
