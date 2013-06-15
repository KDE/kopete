/*
 kopetedbusinterfaceprivate.h - Kopete D-Bus interface private class

 Copyright (c) 2008      by Dennis Nienhüser <earthwings@gentoo.org>
 
 Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

 *************************************************************************
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU Lesser General Public            *
 * License as published by the Free Software Foundation; either          *
 * version 2 of the License, or (at your option) any later version.      *
 *                                                                       *
 *************************************************************************
 */

#ifndef KOPETE_DBUS_INTERFACE_P_H
#define KOPETE_DBUS_INTERFACE_P_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QTime>

#include "kopetemessage.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"

/**
 * Tracks changes of a metacontact and reports them via signals
 */
class ContactStalker: public QObject
{
    Q_OBJECT
    
public:
    ContactStalker(Kopete::MetaContact *contact);

Q_SIGNALS:
	/** Emitted whenever a property of the tracked contact changed */
    void contactChanged(QString contactId);

private Q_SLOTS:
	void messageAppended( Kopete::Message &message, Kopete::ChatSession *session);

	void slotEmitSignal();

    void slotEmitSignalDelayed();
    
    void slotMetaContactRemoved(Kopete::MetaContact *contact);
    
private:
    Kopete::MetaContact *m_contact;
    
    QTime m_lastChange;
};

/**
 * Tracks changes of all metacontacts and reports them via signals.
 * Contains helper functions for KopeteDBusInterface
 */
class KopeteDBusInterfacePrivate: public QObject
{
Q_OBJECT

public:
    KopeteDBusInterfacePrivate();
    
    QStringList listContact(const QList<Kopete::MetaContact*> &contactList);
    
    Kopete::OnlineStatusManager::Categories status2Value(const QString &status);
    
    /**
     * Tries to locate a meta contact using first the protocol:account:contact
     * triplet, if that fails the meta contact id, lastly the displayName.
     * Returns 0 if nothing is found.
     */
    Kopete::MetaContact *findContact(const QString &nameOrId);
    
Q_SIGNALS:
	/** 
	 * Emitted whenever a contact's property changed 
	 * @param contactId protocol:account:contact triplet of the 
	 *        contact with a property change
	 * */ 
    void contactChanged(QString contactId);

private Q_SLOTS:
    void slotMetaContactAdded(Kopete::MetaContact* contact);

};

#endif // KOPETE_DBUS_INTERFACE_P_H
