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

#include <QDialog>

#include "ui_dlgxmppconsole.h"

class JabberClient;

class dlgXMPPConsole : public QDialog
{
	Q_OBJECT
public:
	explicit dlgXMPPConsole(JabberClient *client, QWidget *parent = nullptr);
	virtual ~dlgXMPPConsole();

public Q_SLOTS:
	void slotIncomingXML(const QString &msg);
	void slotOutgoingXML(const QString &msg);

private Q_SLOTS:
	void slotSend();
	void slotClear();

private:
	Ui::dlgXMPPConsole ui;
	JabberClient *mClient;
};

#endif
