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

#include "kopetemetacontact.h"
#include "irccontact.h"
#include "irccontactmanager.h"

#include "irctransferhandler.h"

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

	connect(Kopete::TransferManager::transferManager(), SIGNAL(accepted(Kopete::Transfer *, const QString &)),
		this, SLOT(transferAccepted(Kopete::Transfer *, const QString&)));
	connect( Kopete::TransferManager::transferManager(), SIGNAL(refused(const Kopete::FileTransferInfo &)),
		this, SLOT(transferRefused(const Kopete::FileTransferInfo &)));
}

void IRCTransferHandler::transferCreated(KIRC::Transfer *t)
{
	kdDebug(14120) << k_funcinfo << endl;

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
		kdDebug(14120) << k_funcinfo << "Unknown transfer type" << endl;
		t->deleteLater();
	}
}

void IRCTransferHandler::transferAccepted(Kopete::Transfer *kt, const QString &file)
{
	kdDebug(14120) << k_funcinfo << endl;

	KIRC::Transfer *t = getKIRCTransfer(kt->info());
	if(t)
	{
		t->setFileName(file);
		connectKopeteTransfer(kt, t);
	}
}
void IRCTransferHandler::transferRefused(const Kopete::FileTransferInfo &info)
{
	kdDebug(14120) << k_funcinfo << endl;

	KIRC::Transfer *t = getKIRCTransfer(info);
	if(t)
	{
		t->deleteLater();
	}
}

void IRCTransferHandler::connectKopeteTransfer(Kopete::Transfer *kt, KIRC::Transfer *t)
{
	kdDebug(14120) << k_funcinfo << endl;

	if(kt && t)
	{
		switch(t->type())
		{
//		case KIRC::Transfer::Chat:
		case KIRC::Transfer::FileOutgoing:
		case KIRC::Transfer::FileIncoming:
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

//		connect(kt , SIGNAL(transferCanceled()),
//			t, SLOT(abort()));
//		connect(kt,  SIGNAL(destroyed()),
//			t, SLOT(slotKopeteTransferDestroyed()));

		connect(kt, SIGNAL(result(KIO::Job *)),
			this , SLOT(kioresult(KIO::Job *)));

		t->initiate();
	}
}

void IRCTransferHandler::kioresult(KIO::Job *job)
{
	Kopete::Transfer *kt= (Kopete::Transfer *)job; // FIXME: move to *_cast
	if(!kt)
	{
		kdDebug(14120) << k_funcinfo << "Kopete::Transfer not found from kio:" << job << endl;
		return;
	}

	switch(kt->error())
	{
		case 0:	// 0 means no error
			break;
		case KIO::ERR_USER_CANCELED:
			kdDebug(14120) << k_funcinfo << "User canceled transfer." << endl;
			// KIO::buildErrorString form error don't provide a result string ...
//			if (t->)
//				kt->userAbort(i18n("User canceled transfer."));
//			else
//				kt->userAbort(i18n("User canceled transfer for file:%1").arg(t->fileName()));
			break;
		default:
			kdDebug(14120) << k_funcinfo << "Transfer halted:" << kt->error() << endl;
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

#include "irctransferhandler.moc"
