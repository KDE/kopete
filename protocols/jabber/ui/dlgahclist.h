 /*
    Copyright (c) 2008 by Igor Janssen  <alaves17@gmail.com>

    Kopete    (c) 2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#ifndef DLGAHCLIST_H
#define DLGAHCLIST_H

#include <KDialog>
#include "xmpp_jid.h"
#include "xmpp_client.h"

class AHCommand;
class JabberXDataWidget;
class QVBoxLayout;
class QRadioButton;

class dlgAHCList : public KDialog
{
	Q_OBJECT
public:
	dlgAHCList(const XMPP::Jid &jid, XMPP::Client *client, QWidget *parent = 0);
	~dlgAHCList();

protected slots:
	void slotGetList();
	void slotListReceived();
	void slotExecuteCommand();
	void slotCommandExecuted();

private:
	struct Item
	{
		QRadioButton *radio;
		QString jid;
		QString node;
	};
	XMPP::Jid       mJid;
	XMPP::Client   *mClient;
	QWidget        *mCommandsWidget;
	QVBoxLayout    *mCommandsLayout;
	QList<Item>     mCommands;
};

#endif
