/*
    kopeteblacklister.h - Kopete BlackLister

    Copyright (c) 2004	    by Roie Kerstein	     <sf_kersteinroie@bezeqint.net>

    *************************************************************************
    *									    *
    * This library is free software; you can redistribute it and/or	    *
    * modify it under the terms of the GNU Lesser General Public	    *
    * License as published by the Free Software Foundation; either	    *
    * version 2 of the License, or (at your option) any later version.	    *
    *									    *
    *************************************************************************
*/
#ifndef KOPETEKOPETEBLACKLISTER_H
#define KOPETEKOPETEBLACKLISTER_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include "kopetecontact.h"
#include "kopeteaccount.h"

namespace Kopete {

/**
 * @author Roie Kerstein <sf_kersteinroie@bezeqint.net>
 *
 * The Kopete::BlackLister class manages the list of contacts 
 * the user wishes to ignore permanently. 
 * In order to use the this class, there is no need to create an instance. 
 * Use the @ref Kopete::Account::blacklister() instead. 
 * See also @ref Kopete::Account::block() and @ref Kopete::Account::unblock() .
 * Keep in mind that this class does not discard messages from blocked 
 * users - It only manages the list. It is the up to the protocol to 
 * check wether a user is blocked, and act accordingly. A protocol may 
 * re-implement @ref Kopete::Account::block() and @ref Kopete::Account::unblock() , 
 * and use @ref Kopete::Account::blackLister() as a persistent list manager only, 
 * or connect the signals @ref contactAdded() and @ref contactRemoved() to his slots.
 */


class BlackLister : public QObject
{
Q_OBJECT
public:
    
    /**
     * Constructor:
     * Create an instance, and read from disk the blacklist, if it exists.
     * @param protocolId is the ID of the protocol owning accountId
     * @param accountId is the ID of the owning Account.
     */
    BlackLister(QString protocolId, QString accountId, QObject *parent = 0, const char *name = 0);

    ~BlackLister();

    /**
     * \return whether contact is blocked.
     */
    bool isBlocked(Contact *contact);

    /**
     * \return whether contactId is blocked.
     */
    bool isBlocked(QString &contactId);

public slots:

    /**
     * Add a contact to the blacklist. 
     * This function emits the @ref contactAdded() signal.
     * @param contactId is the ID of the contact to be added to the list.
     */
    void slotAddContact(QString &contactId);

    /**
     * Overloaded function provided for convinience. Behaves exactly like the above.
     */
    void slotAddContact(Contact *contact);

    /**
     * Remove a contact from the blacklist.
     * This function emits the @ref contactRemoved() signal.
     * @param contact is the contact to be removed from the list.
     */
    void slotRemoveContact(Contact *contact);

    /**
     * Overloaded function provided for convinience. Behaves exactly like the above.
     */
    void slotRemoveContact(QString &contactId);

signals:

    /**
     * \brief A new contact has been added to the list
     * Connect to this signal if you want to perform additional actions,
     * and you prefer not to derieve from this class.
     */
    void contactAdded(QString &contactId);

    /**
     * \brief A contact has been removed from the list
     * Connect to this signal if you want to perform additional actions,
     * and you prefer not to derieve from this class.
     */
    void contactRemoved(QString &contactId);

private:
    QStringList m_blacklist;
    QString m_owner;
    QString m_protocol;

};

};

#endif
