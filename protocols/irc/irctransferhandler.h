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

class KopeteTransfer;

class KIRCTransfer;
class KIRCTransferHandler;

class IRCTransferHandler
	: public QObject
{
	Q_OBJECT

public:
	static IRCTransferHandler *self()
		{ return &sm_self; }

private slots:
	void transferCreated(KIRCTransfer *);
	void transferAccepted(KopeteTransfer *kt, const QString&file);
	void transferRefused(const KopeteFileTransferInfo &info);

private:
	IRCTransferHandler();

	void connectKopeteTransfer(KopeteTransfer *kt, KIRCTransfer *t);

	/* warning: After calling this method the KIRCTransfer is removed from the m_idMap.
	 */
	KIRCTransfer *getKIRCTransfer(const KopeteFileTransferInfo &info);

	KIRCTransferHandler *handler();

	static IRCTransferHandler sm_self;
	QIntDict<KIRCTransfer> m_idMap;
};

#endif
