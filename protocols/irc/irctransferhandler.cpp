/*
    irctransferhandler.cpp - IRC transfer.

    Copyright (c) 2004      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include <kdebug.h>

#include "kopetetransfermanager.h"

#include "libkirc/kirctransfer.h"
#include "libkirc/kirctransferhandler.h"

#include "irctransferhandler.h"

IRCTransferHandler IRCTransferHandler::sm_self;

IRCTransferHandler::IRCTransferHandler()
{
	connect(handler(), SIGNAL(transferCreated(KIRCTransfer *)),
		this, SLOT(transferCreated(KIRCTransfer *)));

	connect(KopeteTransferManager::transferManager(), SIGNAL(accepted( KopeteTransfer *, const QString&)),
		this, SLOT(transferAccepted(KopeteTransfer *, const QString&)));
	connect( KopeteTransferManager::transferManager(), SIGNAL(refused(const KopeteFileTransferInfo &)),
		this, SLOT(slotFileTransferRefused(const KopeteFileTransferInfo &)));
}

void IRCTransferHandler::transferCreated(KIRCTransfer *t)
{
	KopeteContact *contact = 0L; // protocol()->getAccountByEngine(t->engine())->getEntityByName(t->nick());
	QString fileName = t->fileName();
	unsigned long fileSize = t->fileSize();

	switch( t->type() )
	{
//	case KIRCTransfer::Chat:
	case KIRCTransfer::FileOutgoing:
		{
			KopeteTransfer *kt = KopeteTransferManager::transferManager()->addTransfer(
				contact, fileName, fileSize, QString::null/*contact->displayName()*/,
				KopeteFileTransferInfo::Outgoing);
			connectKopeteTransfer(kt, t);
		}
		break;
	case KIRCTransfer::FileIncoming:
		{
			int ID = KopeteTransferManager::transferManager()->askIncomingTransfer(
				contact , fileName, fileSize);
			m_idMap.insert(ID, t);
		}
		break;
	default:
		kdDebug(14120) << k_funcinfo << "Unknown transfer type" << endl;
		t->deleteLater();
	}
}

void IRCTransferHandler::transferAccepted(KopeteTransfer *kt, const QString&file)
{
	KIRCTransfer *t = getKIRCTransfer(kt->info());
//	t->setFileName(file);
	connectKopeteTransfer(kt, t);
}
void IRCTransferHandler::transferRefused(const KopeteFileTransferInfo &info)
{
	KIRCTransfer *t = getKIRCTransfer(info);
	t->deleteLater();
}

void IRCTransferHandler::connectKopeteTransfer(KopeteTransfer *kt, KIRCTransfer *t)
{
	if(kt && t)
	{
		connect(kt , SIGNAL(transferCanceled()), t, SLOT(abort()));
		connect(kt,  SIGNAL(destroyed()) , t, SLOT(slotKopeteTransferDestroyed()));

		t->initiate();
	}
}

KIRCTransfer *IRCTransferHandler::getKIRCTransfer(const KopeteFileTransferInfo &info)
{
	KIRCTransfer *t = m_idMap[info.transferId()];
	m_idMap.remove(info.transferId());
	return t;
}

KIRCTransferHandler *IRCTransferHandler::handler()
{
	return KIRCTransferHandler::self();
}

#include "irctransferhandler.moc"
