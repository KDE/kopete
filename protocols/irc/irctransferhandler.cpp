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

#include <kopetetransfermanager.h>

#include "libkirc/kirctransfer.h"
#include "libkirc/kirctransferhandler.h"

#include "irccontact.h"
#include "irccontactmanager.h"

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
	IRCContact *contact = IRCContactManager::existContact(t->engine(), t->nick());
	QString fileName = t->fileName();
	unsigned long fileSize = t->fileSize();

	if(!contact)
	{
		kdDebug(14120) << k_funcinfo << "Trying to create transfer for a non existing contact(" << t->nick() << ")." << endl;
		return;
	}

	switch(t->type())
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
	t->setFileName(file);
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
//		connect(kt , SIGNAL(transferCanceled()), t, SLOT(abort()));
//		connect(kt,  SIGNAL(destroyed()) , t, SLOT(slotKopeteTransferDestroyed()));

		switch(t->type())
		{
//		case KIRCTransfer::Chat:
		case KIRCTransfer::FileOutgoing:
		case KIRCTransfer::FileIncoming:
			connect(t , SIGNAL(fileSizeAcknowledge(unsigned int)),
				kt, SLOT(slotProcessed(unsigned int)));
			break;
		default:
			kdDebug(14120) << k_funcinfo << "Unknown transfer connections for type" << endl;
			t->deleteLater();
			return;
		}

		connect(t , SIGNAL(complete()),
			kt, SLOT(slotComplete()));

		connect(kt, SIGNAL(transferCanceled()),
			t , SLOT(userCancel())); // Should be the following ...
//		connect(kt, SIGNAL(result(KIO *)),
//			this , SLOT(kioresult(KIO *)));

		t->initiate();
	}
}
/*
void IRCTransferHandler::kioresult(KIO *kio)
{
	KIRCTransfer *t = m_transfers[kio];

	if(!t)
	{
		kdDebug(14120) << k_funcinfo << "Transfer not found from kio:" << kio << endl;
		return;
	}

	switch(kio->error())
	{
		case 0:	// 0 means no error
			break;
		case ERR_USER_CANCELLED:
			// KIO::buildErrorString form error don't provide a result string ...
			if()
				t->userAbort(i18n("User canceled transfer."));
			else
				t->userAbort(i18n("User canceled transfer for file:%1").arg(t->fileName()));
			break;
		default:
			t->userAbort(KIO::buildErrorString(kio->error(),t->fileName()));
	}
}
*/
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
