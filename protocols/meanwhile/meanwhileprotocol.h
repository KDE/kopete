/*
    meanwhileprotocl.h - the meanwhile protocol definition

    Copyright (c) 2003-2004 by Sivaram Gottimukkala  <suppandi@gmail.com>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef MEANWHILEPROTOCOL_H
#define MEANWHILEPROTOCOL_H

#include <kopeteprotocol.h>

#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "addcontactpage.h"

#include <kdebug.h>
#define MEANWHILE_DEBUG 14200
#define HERE kdDebug(MEANWHILE_DEBUG) << k_funcinfo << endl
#define mwDebug() kdDebug(MEANWHILE_DEBUG)

class MeanwhileAccount;
class MeanwhileEditAccountWidget;
class MeanwhileAddContactPage;

class MeanwhileProtocol : public Kopete::Protocol
{
    Q_OBJECT
public:
/* const */
    MeanwhileProtocol(QObject *parent, 
                      const char *name, 
                      const QStringList &args);
/* destructor */
    ~MeanwhileProtocol();
    
    virtual AddContactPage * createAddContactWidget( 
                                    QWidget *parent, 
                                    Kopete::Account *account );
                                    
    virtual KopeteEditAccountWidget * createEditAccountWidget( 
                                    Kopete::Account *account, 
                                    QWidget *parent );     

    virtual Kopete::Account * createNewAccount( 
                                    const QString &accountId );                               

    virtual Kopete::Contact *deserializeContact(
                Kopete::MetaContact *metaContact,
                const QMap<QString,QString> &serializedData,
                const QMap<QString, QString> &addressBookData );

    /* kopete doesnt know about these funcs */
    static MeanwhileProtocol *protocol();

    const Kopete::OnlineStatus meanwhileOffline;
    const Kopete::OnlineStatus meanwhileOnline;
    const Kopete::OnlineStatus meanwhileAway;
    const Kopete::OnlineStatus meanwhileBusy;
    const Kopete::OnlineStatus meanwhileIdle;
    const Kopete::OnlineStatus meanwhileUnknown;

    const Kopete::ContactPropertyTmpl statusMessage;
    const Kopete::ContactPropertyTmpl awayMessage;

protected:
    static MeanwhileProtocol *s_protocol;
};

#endif
