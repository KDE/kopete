
/***************************************************************************
                   jabberchooseserver.h  -  Server list for Jabber
                             -------------------
    begin                : Mon Jul 12 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>

		Kopete (C) 2001-2004 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBERCHOOSESERVER_H
#define JABBERCHOOSESERVER_H

#include <kdialogbase.h>
#include <qcstring.h>

class JabberRegisterAccount;
class DlgJabberChooseServer;

namespace KIO
{
	class Job;
	class TransferJob;
}

/**
@author Kopete Developers
*/
class JabberChooseServer : public KDialogBase
{

Q_OBJECT

public:
	JabberChooseServer ( JabberRegisterAccount *parent = 0, const char *name = 0);

	~JabberChooseServer();

private slots:
	void slotOk ();
	void slotCancel ();
	void slotTransferData ( KIO::Job *job, const QByteArray &data );
	void slotTransferResult ( KIO::Job *job );
	void slotSetSelection ( int row );

private:
	DlgJabberChooseServer *mMainWidget;
	JabberRegisterAccount *mParentWidget;
	KIO::TransferJob *mTransferJob;
	QByteArray xmlServerList;

	int mSelectedRow;

};

#endif
