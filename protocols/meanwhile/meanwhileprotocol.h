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

class MeanwhileAccount;
class MeanwhileEditAccountWidget;
class MeanwhileAddContactPage;

class MeanwhileProtocol : public KopeteProtocol
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
                                    KopeteAccount *account );
                                    
    virtual KopeteEditAccountWidget * createEditAccountWidget( 
                                    KopeteAccount *account, 
                                    QWidget *parent );     

    virtual KopeteAccount * createNewAccount( 
                                    const QString &accountId );                               

    virtual KopeteContact *deserializeContact( KopeteMetaContact *metaContact,
                     const QMap<QString,QString> &serializedData,
                     const QMap<QString, QString> &addressBookData );

/* kopete doesnt know about these funcs */
    static MeanwhileProtocol *protocol();

    const KopeteOnlineStatus meanwhileOffline;
    const KopeteOnlineStatus meanwhileOnline;
    const KopeteOnlineStatus meanwhileAway;
    const KopeteOnlineStatus meanwhileBusy;
    const KopeteOnlineStatus meanwhileIdle;
    const KopeteOnlineStatus meanwhileUnknown;

    const Kopete::ContactPropertyTmpl statusMessage;
    const Kopete::ContactPropertyTmpl awayMessage;

protected:
	static MeanwhileProtocol *s_protocol;
};

#endif
