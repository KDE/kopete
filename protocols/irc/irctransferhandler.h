/*
    irctransferhandler.h - IRC transfer.

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCTRANSFERHANDLER_H
#define IRCTRANSFERHANDLER_H

#include <qintdict.h>

#include <kopetetransfermanager.h>

namespace Kopete
{
	class Transfer;
}

namespace KIRC
{
class Transfer;
class TransferHandler;
}

class IRCTransferHandler
	: public QObject
{
	Q_OBJECT

public:
	static IRCTransferHandler *self();

private slots:
	void transferCreated(KIRC::Transfer *);
	void transferAccepted(Kopete::Transfer *kt, const QString&file);
	void transferRefused(const Kopete::FileTransferInfo &info);

	void kioresult(KIO::Job *job);

private:
	IRCTransferHandler();

	void connectKopeteTransfer(Kopete::Transfer *kt, KIRC::Transfer *t);

	/* warning: After calling this method the KIRC::Transfer is removed from the m_idMap.
	 */
	KIRC::Transfer *getKIRCTransfer(const Kopete::FileTransferInfo &info);

	KIRC::TransferHandler *handler();

	QIntDict<KIRC::Transfer> m_idMap;
};

#endif
