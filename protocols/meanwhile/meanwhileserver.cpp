/*
    meanwhileserver.cpp - server for an account.

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
#include <qstring.h>
#include <klocale.h>
#include "meanwhilelibrary.h"
#include "meanwhileprotocol.h"
#include "meanwhileserver.h"
#include "meanwhilecontact.h"

#define HERE 

#define MEANWHILESERVER_CPP 1
MeanwhileServer::MeanwhileServer(QString server, int port)
{
HERE
    library = new MeanwhileLibrary(server,port); 
    if (library->bad())
    {
        delete library;
        library = NULL;
    }
    else
    {
        QObject::connect(library, 
                     SIGNAL(loginDone()),
                     SLOT(slotLoginDone())); 
        QObject::connect(library, 
                     SIGNAL(connectionLost()),
                     SLOT(slotConnectionLost())); 
        QObject::connect(library, 
                     SIGNAL(serverNotificationReceived(const QString&)),
                     SLOT(slotServerNotification(const QString&))); 
        QObject::connect(library, 
                     SIGNAL(userStatusChanged(
                                const QString &,
                                bool,
                                time_t,
                                int,
                                const QString &)),
                     SLOT(slotUserStatusChanged(
                                const QString &,
                                bool,
                                time_t,
                                int,
                                const QString &)));
        QObject::connect(library, 
                     SIGNAL(mesgReceived(
                                const QString &,
                                const QString &)),
                     SLOT(slotMesgReceived(
                                const QString &,
                                const QString &)));
        QObject::connect(library, 
                     SIGNAL(userTyping(
                                const QString &,
                                bool)),
                     SLOT(slotUserTyping(
                                const QString &,
                                bool)));
    }
}

MeanwhileServer::~MeanwhileServer()
{
HERE
    delete library;
}

bool MeanwhileServer::bad()
{
HERE
    return (library == NULL);
}

int MeanwhileServer::login(QString username, QString passwd)
{
HERE
    library->login(username,passwd);
    return 0;
}

void MeanwhileServer::logoff()
{
HERE
    library->logoff();
}

void MeanwhileServer::slotLoginDone()
{
HERE
    emit loginDone();
}

void MeanwhileServer::slotServerNotification(const QString &errMesg)
{
    emit notificationAvailable(errMesg);
}

void MeanwhileServer::slotConnectionLost()
{
    emit connectionLost();
}

void MeanwhileServer::addContacts(const QDict<KopeteContact>& _contacts)
{
    library->addContacts(contacts=_contacts);
}

void MeanwhileServer::addContact(KopeteContact *_contact,
                                const QDict<KopeteContact>& _contacts)
{
    contacts = _contacts;
    MeanwhileContact *contact = static_cast<MeanwhileContact *>(_contact);
    library->addContact(contact->meanwhileId);
}

int MeanwhileServer::sendIm(KopeteContact *_contact, const QString &msg)
{
    MeanwhileContact *contact = static_cast<MeanwhileContact *>(_contact);
    return library->sendIm(contact->meanwhileId,msg);
}

void MeanwhileServer::sendTyping(KopeteContact *_contact, bool isTyping)
{
    MeanwhileContact *contact = static_cast<MeanwhileContact *>(_contact);
    library->sendTyping(contact->meanwhileId,isTyping);
}

void MeanwhileServer::goAway(const QString &awayMsg)
{
    library->setState(MeanwhileLibrary::Away,awayMsg);
}

void MeanwhileServer::goDND(const QString &dndMsg)
{
    library->setState(MeanwhileLibrary::Busy,dndMsg);
}

void MeanwhileServer::changeStatus(const QString &statusMesg)
{
    library->setStatusMesg(statusMesg);
}

void MeanwhileServer::slotMesgReceived(const QString &fromUser,
                                       const QString &mesg)
{
    emit mesgReceived(fromUser,mesg);
}

void MeanwhileServer::slotUserTyping(const QString &user,
                                     bool isTyping)
{
    emit userTyping(user,isTyping);
}

void MeanwhileServer::goActive(const QString &status)
{
    library->setState(MeanwhileLibrary::Active,status);
}

#define STATUS(VALUE,onlinestatus)                        \
    (status==MeanwhileLibrary::VALUE)?                    \
            MeanwhileProtocol::protocol()->onlinestatus:
void MeanwhileServer::slotUserStatusChanged(
                                const QString & userid,
                                bool isOnline,
                                time_t idletime,
                                int status,
                                const QString & statusDesc)
{
    MeanwhileContact *contact = static_cast<MeanwhileContact *>(contacts[userid]);
    if (contact != NULL)
    {
          contact->setProperty(MeanwhileProtocol::protocol()->statusMessage,
                               statusDesc);
          contact->setProperty(MeanwhileProtocol::protocol()->awayMessage,
                               statusDesc);
          contact->setOnlineStatus( 
                      isOnline?
                         STATUS(Away,meanwhileAway)
                         STATUS(Active,meanwhileOnline)
                         STATUS(Idle,meanwhileIdle)
                         STATUS(Busy,meanwhileBusy)
                         MeanwhileProtocol::protocol()->meanwhileUnknown:
                         MeanwhileProtocol::protocol()->meanwhileOffline);
	   
/*        if ((status == MeanwhileLibrary::Idle) && (idletime != 0))
            contact->setStatusDescription(statusDesc + "["+QString::number(idletime/60)+" mins]");
        else
            contact->setStatusDescription(statusDesc); */
    }
}
#undef STATUS
#include "meanwhileserver.moc"
