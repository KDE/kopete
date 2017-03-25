/*
    irctransferhandler.cpp - IRC transfer.

    Copyright (c) 2004-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2004-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "irctransferhandler.h"
#include "irccontact.h"

#include "kirctransfer.h"
#include "kirctransferhandler.h"

#include "kopetemetacontact.h"

#include <kopetetransfermanager.h>

#include <kdebug.h>

IRCTransferHandler *IRCTransferHandler::self()
{
    static IRCTransferHandler sm_self;
    return &sm_self;
}

KIRC::TransferHandler *IRCTransferHandler::handler()
{
    return KIRC::TransferHandler::self();
}

IRCTransferHandler::IRCTransferHandler()
{
    connect(handler(), SIGNAL(transferCreated(KIRC::Transfer *)),
            this, SLOT(transferCreated(KIRC::Transfer *)));

    connect(Kopete::TransferManager::transferManager(), SIGNAL(accepted(Kopete::Transfer *,QString)),
            this, SLOT(transferAccepted(Kopete::Transfer *,QString)));
    connect(Kopete::TransferManager::transferManager(), SIGNAL(refused(Kopete::FileTransferInfo)),
            this, SLOT(transferRefused(Kopete::FileTransferInfo)));
}

void IRCTransferHandler::transferCreated(KIRC::Transfer *t)
{
    /*
       kDebug(14120) ;

       IRCContact *contact = IRCContactManager::existContact(t->engine(), t->nick());
       QString fileName = t->fileName();
       unsigned long fileSize = t->fileSize();

       if(!contact)
       {
           kDebug(14120) << "Trying to create transfer for a non existing contact(" << t->nick() << ").";
           return;
       }

       switch(t->type())
       {
   //	case KIRC::Transfer::Chat:
       case KIRC::Transfer::FileOutgoing:
           {
               Kopete::Transfer *kt = Kopete::TransferManager::transferManager()->addTransfer(
                   contact, fileName, fileSize, contact->metaContact()->displayName(),
                   Kopete::FileTransferInfo::Outgoing);
               connectKopeteTransfer(kt, t);
           }
           break;
       case KIRC::Transfer::FileIncoming:
           {
               int ID = Kopete::TransferManager::transferManager()->askIncomingTransfer(
                   contact , fileName, fileSize);
               m_idMap.insert(ID, t);
           }
           break;
       default:
           kDebug(14120) << "Unknown transfer type";
           t->deleteLater();
       }*/
}

void IRCTransferHandler::transferAccepted(Kopete::Transfer *kt, const QString &file)
{
    kDebug(14120);

    KIRC::Transfer *t = getKIRCTransfer(kt->info());
    if (t) {
        t->setFileName(file);
        connectKopeteTransfer(kt, t);
    }
}

void IRCTransferHandler::transferRefused(const Kopete::FileTransferInfo &info)
{
    kDebug(14120);

    KIRC::Transfer *t = getKIRCTransfer(info);
    if (t) {
        t->deleteLater();
    }
}

void IRCTransferHandler::connectKopeteTransfer(Kopete::Transfer *kt, KIRC::Transfer *t)
{
    kDebug(14120);

    if (kt && t) {
        switch (t->type()) {
//		case KIRC::Transfer::Chat:
        case KIRC::Transfer::FileOutgoing:
        case KIRC::Transfer::FileIncoming:
            connect(t, SIGNAL(fileSizeAcknowledge(uint)),
                    kt, SLOT(slotProcessed(uint)));
            break;
        default:
            kDebug(14120) << "Unknown transfer connections for type";
            t->deleteLater();
            return;
        }

        connect(t, SIGNAL(complete()),
                kt, SLOT(slotComplete()));

//		connect(kt , SIGNAL(transferCanceled()),
//			t, SLOT(abort()));
//		connect(kt,  SIGNAL(destroyed()),
//			t, SLOT(slotKopeteTransferDestroyed()));

        connect(kt, SIGNAL(result(KJob *)),
                this, SLOT(kioresult(KJob *)));

        t->initiate();
    }
}

void IRCTransferHandler::kioresult(KJob *job)
{
    Kopete::Transfer *kt = (Kopete::Transfer *)job; // FIXME: move to *_cast
    if (!kt) {
        kDebug(14120) << "Kopete::Transfer not found from kio:" << job;
        return;
    }

    switch (kt->error()) {
    case 0:     // 0 means no error
        break;
    case KIO::ERR_USER_CANCELED:
        kDebug(14120) << "User canceled transfer.";
        // KIO::buildErrorString form error don't provide a result string ...
//			if (t->)
//				kt->userAbort(i18n("User canceled transfer."));
//			else
//				kt->userAbort(i18n("User canceled transfer for file:%1").arg(t->fileName()));
        break;
    default:
        kDebug(14120) << "Transfer halted:" << kt->error();
//			kt->userAbort(KIO::buildErrorString(kt->error(), kt->fileName()));
        break;
    }
}

KIRC::Transfer *IRCTransferHandler::getKIRCTransfer(const Kopete::FileTransferInfo &info)
{
    KIRC::Transfer *t = m_idMap[info.transferId()];
    m_idMap.remove(info.transferId());
    return t;
}
