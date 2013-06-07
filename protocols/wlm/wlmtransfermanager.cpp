/*
    wlmtransfermanager.cpp - Kopete Wlm Protocol

    Copyright (c) 2008      by Tiago Salem Herrmann <tiagosh@gmail.com>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "wlmtransfermanager.h"
#include "wlmcontact.h"
#include "kopetecontact.h"
#include "kopeteuiglobal.h"
#include "kopetetransfermanager.h"

#include <QObject>
#include <kmessagebox.h>
#include <kcodecs.h>
#include <klocale.h>

#include <kdebug.h>

WlmTransferManager::WlmTransferManager (WlmAccount * account1) :
m_account (account1),
nextID (1)
{
    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (incomingFileTransfer
                              (MSN::SwitchboardServerConnection *,
                               const MSN::fileTransferInvite &)), this,
                      SLOT (incomingFileTransfer
                            (MSN::SwitchboardServerConnection *,
                             const MSN::fileTransferInvite &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (gotFileTransferProgress
                              (MSN::SwitchboardServerConnection *, 
                               const unsigned int &,
                               const unsigned long long &)), this,
                      SLOT (gotFileTransferProgress
                            (MSN::SwitchboardServerConnection *,
                             const unsigned int &,
                             const unsigned long long &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (gotFileTransferFailed (MSN::SwitchboardServerConnection *,
                              const unsigned int &, const MSN::fileTransferError&)),
                      this,
                      SLOT (gotFileTransferFailed (MSN::SwitchboardServerConnection *,
                              const unsigned int &, const MSN::fileTransferError&)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (gotFileTransferSucceeded
                              (MSN::SwitchboardServerConnection *,
                               const unsigned int &)), this,
                      SLOT (gotFileTransferSucceeded (MSN::SwitchboardServerConnection *,
                              const unsigned int &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (slotfileTransferInviteResponse
                              (MSN::SwitchboardServerConnection *,
                               const unsigned int &, const bool &)), this,
                      SLOT (fileTransferInviteResponse
                            (MSN::SwitchboardServerConnection * ,
                             const unsigned int &, const bool &)));

    connect (Kopete::TransferManager::transferManager (),
             SIGNAL (accepted(Kopete::Transfer*,QString)),
             this, SLOT (slotAccepted(Kopete::Transfer*,QString)));
    connect (Kopete::TransferManager::transferManager (),
             SIGNAL (refused(Kopete::FileTransferInfo)),
             this, SLOT (slotRefused(Kopete::FileTransferInfo)));
}

void
WlmTransferManager::fileTransferInviteResponse (MSN::SwitchboardServerConnection * /*conn*/,
                                                const unsigned int &sessionID,
                                                const bool & response)
{
    if(!transferSessions.count(sessionID))
        return;
 
    if (response)
    {
        transferSessionData tfd = transferSessions[sessionID];
        Kopete::ContactPtrList chatmembers;
	    chatmembers.append (account ()->contacts().value(tfd.to));
        WlmChatSession *_manager =
            qobject_cast <WlmChatSession *>(Kopete::ChatSessionManager::self ()->
              findChatSession (account ()->myself (), chatmembers,
                               account ()->protocol ()));
        if (!_manager)
        {
            _manager =
                new WlmChatSession (account ()->protocol (),
                                    account ()->myself (), chatmembers);
        }
    }
    else
    {
        transferSessionData tfd = transferSessions[sessionID];
        if(tfd.internalID)
            Kopete::TransferManager::transferManager()->
                cancelIncomingTransfer(tfd.internalID);
        else
            tfd.ft->slotError(KIO::ERR_ABORTED, i18n("File transfer cancelled."));
    }
}

WlmTransferManager::~WlmTransferManager ()
{
}

void
WlmTransferManager::incomingFileTransfer (MSN::SwitchboardServerConnection * conn,
                                          const MSN::fileTransferInvite & ft)
{
    QString passport = WlmUtils::passport(ft.userPassport);
    Kopete::Contact * contact = account ()->contacts().value(passport);

    if(!contact)
        return;

    if (ft.type == MSN::FILE_TRANSFER_WITH_PREVIEW
        || ft.type == MSN::FILE_TRANSFER_WITHOUT_PREVIEW)
    {
        QPixmap preview;
        if (ft.type == MSN::FILE_TRANSFER_WITH_PREVIEW)
        {
            preview.loadFromData (KCodecs::base64Decode (ft.preview.c_str()));
        }
        transferSessionData tsd;
        tsd.from = passport;
        tsd.to = account ()->myself ()->contactId ();
        tsd.ft = NULL;
        tsd.internalID = 0;
        account ()->chatManager ()->createChat (conn);
        WlmChatSession *chat = account ()->chatManager ()->chatSessions[conn];
        if(chat)
            chat->setCanBeDeleted (false);

        tsd.internalID = Kopete::TransferManager::transferManager()->askIncomingTransfer(contact,
                WlmUtils::utf8(ft.filename), ft.filesize, "", QString::number (ft.sessionId), preview);
        transferSessions[ft.sessionId] = tsd;
    }
}

void
WlmTransferManager::gotFileTransferProgress (MSN::SwitchboardServerConnection * /*conn*/,
                                             const unsigned int &sessionID,
                                             const unsigned long long
                                             &transferred)
{
    if(!transferSessions.count(sessionID))
        return;

    Kopete::Transfer * transfer = transferSessions[sessionID].ft;
    if (transfer)
        transfer->slotProcessed (transferred);
}

void
WlmTransferManager::slotAccepted (Kopete::Transfer * ft,
                                  const QString & filename)
{
    Kopete::ContactPtrList chatmembers;

    // grab contactId from the sender
    unsigned int sessionID = ft->info ().internalId ().toUInt ();

    if(!transferSessions.count(sessionID))
        return;
    
    QString from = transferSessions[sessionID].from;

    if (from.isEmpty ())
        return;

    // find an existent session, or create a new one
	chatmembers.append (account ()->contacts().value(from));
    WlmChatSession *_manager = qobject_cast <WlmChatSession *>
        (Kopete::ChatSessionManager::self ()->
          findChatSession (account ()->myself (), chatmembers,
                           account ()->protocol ()));

    if (!_manager)
    {
        _manager =
            new WlmChatSession (account ()->protocol (),
                                account ()->myself (), chatmembers);
    }

    MSN::SwitchboardServerConnection * conn = _manager->getChatService ();
    if (!conn)
        return;

    _manager->setCanBeDeleted (false);
    transferSessions[sessionID].ft = ft;

    connect (ft, SIGNAL (transferCanceled()), this, SLOT (slotCanceled()));

    conn->fileTransferResponse (sessionID, QFile::encodeName(filename).constData (), true);
}

void
WlmTransferManager::slotRefused (const Kopete::FileTransferInfo & fti)
{
    Kopete::ContactPtrList chatmembers;
    chatmembers.append (fti.contact ());
    WlmChatSession *_manager = qobject_cast <WlmChatSession *>
        (Kopete::ChatSessionManager::self ()->
          findChatSession (account ()->myself (), chatmembers,
                           account ()->protocol ()));

    if (!_manager)
        return;

    MSN::SwitchboardServerConnection * conn = _manager->getChatService ();
    if (!conn)
        return;

    conn->fileTransferResponse (fti.internalId ().toUInt (), "", false);
}

void
WlmTransferManager::gotFileTransferFailed (MSN::SwitchboardServerConnection * /*conn*/,
                                            const unsigned int &sessionID,
                                            const MSN::fileTransferError & /*error*/)
{
    if(!transferSessions.count(sessionID))
        return;

    transferSessionData tsd = transferSessions[sessionID];
    if (tsd.internalID)
    {
        Kopete::TransferManager::transferManager ()->
            cancelIncomingTransfer(tsd.internalID);
        if(tsd.ft)
            tsd.ft->slotError(KIO::ERR_ABORTED, i18n("File transfer cancelled."));
    }
    else
    {
        if (tsd.ft)
        {
            tsd.ft->slotError(KIO::ERR_ABORTED, i18n("File transfer cancelled."));
        }
    }
    transferSessions.remove (sessionID);

}

void
WlmTransferManager::gotFileTransferSucceeded (MSN::SwitchboardServerConnection * /*conn*/,
                                            const unsigned int &sessionID)
{
    Kopete::Transfer * transfer = transferSessions[sessionID].ft;
    if (transfer)
    {
        Kopete::ContactPtrList chatmembers;
        if (transfer->info ().direction () ==
            Kopete::FileTransferInfo::Incoming)
            chatmembers.append (account ()->contacts().value(transferSessions[sessionID].from));
        else
	        chatmembers.append (account ()->contacts().value(transferSessions[sessionID].to));

        WlmChatSession *_manager = qobject_cast <WlmChatSession *>
            (Kopete::ChatSessionManager::self ()->
              findChatSession (account ()->myself (), chatmembers,
                               account ()->protocol ()));
        if (_manager)
            _manager->raiseView ();
        transfer->slotComplete ();
        transferSessions.remove (sessionID);
    }
}

void
WlmTransferManager::slotCanceled ()
{
    kDebug (14210) << k_funcinfo;
    Kopete::Transfer * ft = qobject_cast < Kopete::Transfer * >(sender ());
    if (!ft)
        return;
    unsigned int sessionID = 0;
    QMap < unsigned int, transferSessionData >::iterator i = 
        transferSessions.begin ();
    for (; i != transferSessions.end (); ++i)
        if (i.value ().ft == ft)
            sessionID = i.key ();

    if (!sessionID)
        return;

    transferSessionData session = transferSessions[sessionID];

    Kopete::ContactPtrList chatmembers;
    if (ft->info ().direction () == Kopete::FileTransferInfo::Incoming)
	    chatmembers.append (account ()->contacts().value(session.from));
    else
	    chatmembers.append (account ()->contacts().value(session.to));

    WlmChatSession *_manager = qobject_cast <WlmChatSession *>
        (Kopete::ChatSessionManager::self ()->
          findChatSession (account ()->myself (), chatmembers,
                           account ()->protocol ()));

    if (!_manager)
        return;
    _manager->raiseView ();

    MSN::SwitchboardServerConnection * conn = _manager->getChatService ();

    if (!conn)
        return;

    if (sessionID)
    {
        transferSessions.remove (sessionID);
        conn->cancelFileTransfer (sessionID);
    }
}

#include "wlmtransfermanager.moc"
