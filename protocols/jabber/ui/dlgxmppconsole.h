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

#ifndef DLGXMPPCONSOLE_H
#define DLGXMPPCONSOLE_H

#include "ui_dlgxmppconsole.h"

class JabberClient;

class dlgXMPPConsole : public KDialog
{
	Q_OBJECT
public:
	explicit dlgXMPPConsole(JabberClient *client, QWidget *parent = 0);
	virtual ~dlgXMPPConsole();

public slots:
	void slotIncomingXML(const QString &msg);
	void slotOutgoingXML(const QString &msg);

private slots:
	void slotSend();
	void slotClear();

private:
	Ui::dlgXMPPConsole ui;
	JabberClient *mClient;
};

#endif
