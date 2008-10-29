/*
    networkconfigwidget.h - IRC Network configurator widget.

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCNETWORKCONFIGWIDGET_H
#define IRCNETWORKCONFIGWIDGET_H

#include <QDialog>

#include "ui_networkconfig.h"

#include "ircnetwork.h"

class IRCNetworkConfigWidget
	: public QDialog, public Ui::NetworkConfig
{
	Q_OBJECT

public:
	explicit IRCNetworkConfigWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~IRCNetworkConfigWidget();

	void editNetworks( const QString &networkName = QString() );

signals:
	void networkSelected(const IRC::Network &network);

	void networkConfigUpdated(const QString &selectedNetwork);

private slots:
	// FIXME: All the code for managing the networks list should be in another class - Will
	void slotUpdateNetworkConfig();
	void slotUpdateNetworkHostConfig();
	void slotMoveServerUp();
	void slotMoveServerDown();
	void slotDeleteNetwork();
	void slotDeleteHost();
	void slotNewNetwork();
	void slotRenameNetwork();
	void slotNewHost();
	void slotHostPortChanged( int value );
	// end of network list specific code
	
	// copies the altered information to the global IRCNetworks List
	void slotSaveNetworkConfig();

private:
	void storeCurrentNetwork();
	void storeCurrentHost();

	class Private;
	Private * const d;
};

#endif
