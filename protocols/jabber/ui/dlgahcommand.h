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

#ifndef DLGAHCOMMAND_H
#define DLGAHCOMMAND_H

#include <KDialog>
#include "xmpp_xdata.h"
#include "xmpp_jid.h"
#include "xmpp_client.h"

class AHCommand;
class JabberXDataWidget;

class dlgAHCommand : public KDialog
{
	Q_OBJECT
public:
	dlgAHCommand(const AHCommand &r, const XMPP::Jid &jid, XMPP::Client *client, bool final = false, QWidget *parent = 0);
	~dlgAHCommand();

protected slots:
	void slotPrev();
	void slotNext();
	void slotComplete();
	void slotExecute();
	void slotCancel();

protected:
	XMPP::XData data() const;

private:
	QString mNode;
	QString mSessionId;
	XMPP::Jid mJid;
	XMPP::Client *mClient;
	JabberXDataWidget *mXDataWidget;
};

#endif
