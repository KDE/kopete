/*
    networkconfigwidget.cpp - IRC Network configurator widget.

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "networkconfigwidget.h"
#include "channellist.h"
#include "ircaddcontactpage.h"
#include "ircguiclient.h"
#include "irceditaccountwidget.h"
#include "irctransferhandler.h"

#include "kircengine.h"

#include "kopeteaccountmanager.h"
#include "kopetechatsessionmanager.h"
#include "kopetecommandhandler.h"
#include "kopeteglobal.h"
#include "kopeteonlinestatusmanager.h"
#include "kopeteonlinestatus.h"
#include "kopeteview.h"
#include "kopeteuiglobal.h"

#include <kaction.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <kinputdialog.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kuser.h>

#include <qcheckbox.h>
#include <qdom.h>
#include <qfile.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qvalidator.h>

#include <dom/html_element.h>
#include <unistd.h>

using namespace Kopete;

IRCNetworkConfigWidget::IRCNetworkConfigWidget(QWidget *parent, WFlags flags)
	: NetworkConfig(parent, "network_config", flags)
{
//	kdDebug(14120) << k_funcinfo << endl;

	m_host->setValidator( new QRegExpValidator( QString::fromLatin1("^[\\w-\\.]*$"), this ) );
	upButton->setIconSet( SmallIconSet( "up" )  );
	downButton->setIconSet( SmallIconSet( "down" ) );

	connect(networkList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkConfig()));
	connect(hostList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));

	connect(this, SIGNAL(accepted()),
		this, SLOT(slotSaveNetworkConfig()));
	connect(this, SIGNAL(rejected()),
		this, SLOT(slotReadNetworks()));

	connect(upButton, SIGNAL(clicked()),
		this, SLOT(slotMoveServerUp()));
	connect(downButton, SIGNAL(clicked()),
		this, SLOT(slotMoveServerDown()));

	connect(newNetwork, SIGNAL(clicked()),
		this, SLOT(slotNewNetwork()));
	connect(renameNetwork, SIGNAL(clicked()),
		this, SLOT(slotRenameNetwork()));
	connect(removeNetwork, SIGNAL(clicked()),
		this, SLOT(slotDeleteNetwork()));

	connect(removeHost, SIGNAL(clicked()),
		this, SLOT(slotDeleteHost()));
	connect(newHost, SIGNAL(clicked()),
		this, SLOT(slotNewHost()));

	connect(port, SIGNAL(valueChanged(int)),
		this, SLOT(slotHostPortChanged(int)));
}

IRCNetworkConfigWidget::~IRCNetworkConfigWidget()
{
}

void IRCNetworkConfigWidget::editNetworks(const QString &networkName)
{
	disconnect(networkList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkConfig()));
	disconnect(hostList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));

	networkList->clear();

	for( QDictIterator<IRCNetwork> it( m_networks ); it.current(); ++it )
	{
		IRCNetwork *net = it.current();
		networkList->insertItem( net->name );
	}

	networkList->sort();

	connect(networkList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkConfig()));
	connect(hostList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));

	if( !networkName.isEmpty() )
		networkList->setSelected(networkList->findItem(networkName), true);

	//slotUpdateNetworkConfig(); // unnecessary, setSelected emits selectionChanged
}

void IRCNetworkConfigWidget::slotUpdateNetworkConfig()
{
	// update the data structure of the previous selection from the UI
	storeCurrentNetwork();

	// update the UI from the data for the current selection
	IRCNetwork *net = m_networks[ networkList->currentText() ];
	if( net )
	{
		description->setText( net->description );
		hostList->clear();

		for( QValueList<IRCHost*>::iterator it = net->hosts.begin(); it != net->hosts.end(); ++it )
			hostList->insertItem( (*it)->host + QString::fromLatin1(":") + QString::number((*it)->port) );

		// prevent nested event loop crash
		disconnect(hostList, SIGNAL(selectionChanged()),
			this, SLOT( slotUpdateNetworkHostConfig() ) );

		hostList->setSelected( 0, true );
		slotUpdateNetworkHostConfig();

		connect(hostList, SIGNAL(selectionChanged()),
			this, SLOT(slotUpdateNetworkHostConfig()));
	}

	// record the current selection
	m_uiCurrentNetworkSelection = networkList->currentText();
}

void IRCNetworkConfigWidget::storeCurrentNetwork()
{
	if ( !m_uiCurrentNetworkSelection.isEmpty() )
	{
		IRCNetwork *net = m_networks[ m_uiCurrentNetworkSelection ];
		if ( net )
		{
			net->description = description->text(); // crash on 2nd dialog show here!
		}
		else
			kdDebug( 14020 ) << m_uiCurrentNetworkSelection << " was already gone from the cache!" << endl;
	}
}

void IRCNetworkConfigWidget::storeCurrentHost()
{
	if ( !m_uiCurrentHostSelection.isEmpty()  )
	{
		IRCHost *host = m_hosts[ m_uiCurrentHostSelection ];
		if ( host )
		{
			host->host = m_host->text();
			host->password = password->text();
			host->port = port->text().toInt();
			host->ssl = useSSL->isChecked();
		}
	}
}

void IRCNetworkConfigWidget::slotHostPortChanged( int value )
{
	QString entryText = m_uiCurrentHostSelection + QString::fromLatin1(":") + QString::number( value );
	// changeItem causes a take() and insert, and we don't want a selectionChanged() signal that sets all this off again.
	disconnect(hostList, SIGNAL(selectionChanged()),
		this, SLOT( slotUpdateNetworkHostConfig() ) );

	hostList->changeItem(entryText, hostList->currentItem());

	connect(hostList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));
}

void IRCNetworkConfigWidget::slotUpdateNetworkHostConfig()
{
	storeCurrentHost();

	if (hostList->selectedItem())
	{
		m_uiCurrentHostSelection = hostList->currentText().section(':', 0, 0);
		IRCHost *host = m_hosts[ m_uiCurrentHostSelection ];

		if( host )
		{
			m_host->setText( host->host );
			password->setText( host->password );
			port->setValue( host->port );
			useSSL->setChecked( host->ssl );

			upButton->setEnabled(hostList->currentItem() > 0);
			downButton->setEnabled(hostList->currentItem() < (int)(hostList->count()-1));
		}
	}
	else
	{
		m_uiCurrentHostSelection = QString();
		disconnect(port, SIGNAL(valueChanged(int)),
			this, SLOT( slotHostPortChanged( int ) ) );
		m_host->clear();
		password->clear();
		port->setValue( 6667 );
		useSSL->setChecked( false );
		connect(port, SIGNAL(valueChanged(int)),
			this, SLOT( slotHostPortChanged( int ) ) );
	}
}

void IRCNetworkConfigWidget::slotDeleteNetwork()
{
	QString network = networkList->currentText();
	if( KMessageBox::warningContinueCancel(
		UI::Global::mainWidget(), i18n("<qt>Are you sure you want to delete the network <b>%1</b>?<br>"
		"Any accounts which use this network will have to be modified.</qt>")
		.arg(network), i18n("Deleting Network"),
		KGuiItem(i18n("&Delete Network"),"editdelete"), QString::fromLatin1("AskIRCDeleteNetwork") ) == KMessageBox::Continue )
	{
		IRCNetwork *net = m_networks[ network ];
		for( QValueList<IRCHost*>::iterator it = net->hosts.begin(); it != net->hosts.end(); ++it )
		{
			m_hosts.remove( (*it)->host );
			delete (*it);
		}
		m_networks.remove( network );
		delete net;
		networkList->removeItem(networkList->currentItem());
		slotUpdateNetworkHostConfig();

	}
}

void IRCNetworkConfigWidget::slotDeleteHost()
{
	QString hostName = m_host->text();
	if ( KMessageBox::warningContinueCancel(
		UI::Global::mainWidget(), i18n("<qt>Are you sure you want to delete the host <b>%1</b>?</qt>")
		.arg(hostName), i18n("Deleting Host"),
		KGuiItem(i18n("&Delete Host"),"editdelete"), QString::fromLatin1("AskIRCDeleteHost")) == KMessageBox::Continue )
	{
		IRCHost *host = m_hosts[ hostName ];
		if ( host )
		{
			disconnect(hostList, SIGNAL(selectionChanged()),
				this, SLOT( slotUpdateNetworkHostConfig() ) );

			QString entryText = host->host + QString::fromLatin1(":") + QString::number(host->port);
			QListBoxItem * justAdded = hostList->findItem( entryText );
			hostList->removeItem(hostList->index(justAdded));

			connect(hostList, SIGNAL(selectionChanged()),
				this, SLOT(slotUpdateNetworkHostConfig()));

			// remove from network as well
			IRCNetwork *net = m_networks[ m_uiCurrentNetworkSelection ];
			net->hosts.remove( host );

			m_hosts.remove( host->host );
			delete host;
		}
	}
}

void IRCNetworkConfigWidget::slotNewNetwork()
{
	// create a new network struct
	IRCNetwork *net = new IRCNetwork;
	// give it the name of 'New Network' (incrementing number if needed)
	QString netName = QString::fromLatin1( "New Network" );
	if ( m_networks.find( netName ) )
	{
		int newIdx = 1;
		do {
			netName = QString::fromLatin1( "New Network #%1" ).arg( newIdx++ );
		}
		while ( m_networks.find( netName ) && newIdx < 100 );
		if ( newIdx == 100 ) // pathological case
			return;
	}
	net->name = netName;
	// and add it to the networks dict and list
	m_networks.insert(net->name, net);
	networkList->insertItem(net->name);
	QListBoxItem * justAdded = networkList->findItem(net->name);
	networkList->setSelected(justAdded, true);
	networkList->setBottomItem(networkList->index(justAdded));
}

void IRCNetworkConfigWidget::slotNewHost()
{
	// create a new host
	IRCHost *host = new IRCHost;
	// prompt for a name
	bool ok;
	QString name = KInputDialog::getText(
			i18n("New Host"),
			i18n("Enter the hostname of the new server:"),
			QString::null, &ok, UI::Global::mainWidget() );
	if ( ok )
	{
		// dupe check
		if ( m_hosts[ name ] )
		{
			KMessageBox::sorry(this, i18n( "A host already exists with that name" ) );
			return;
		}
		// set defaults on others
		host->host = name;
		host->port = 6667;
		host->ssl = false;
		// add it to the dict
		m_hosts.insert( host->host, host );
		// add it to the network!
		IRCNetwork *net = m_networks[networkList->currentText() ];
		net->hosts.append( host );
		// add it to the gui
		QString entryText = host->host + QString::fromLatin1(":") + QString::number(host->port);
		hostList->insertItem( entryText );
		// select it in the gui
		QListBoxItem * justAdded = hostList->findItem( entryText );
		hostList->setSelected( justAdded, true );
		//hostList->setBottomItem(hostList->index(justAdded));
	}
}

void IRCNetworkConfigWidget::slotRenameNetwork()
{
	IRCNetwork *net = m_networks[ m_uiCurrentNetworkSelection ];
	if ( net )
	{
		bool ok;
		// popup up a dialog containing the current name
		QString name = KInputDialog::getText(
				i18n("Rename Network"),
				i18n("Enter the new name for this network:"),
				m_uiCurrentNetworkSelection, &ok,
				UI::Global::mainWidget() );
		if ( ok )
		{
			if ( m_uiCurrentNetworkSelection != name )
			{
				// dupe check
				if ( m_networks[ name ] )
				{
					KMessageBox::sorry(this, i18n( "A network already exists with that name" ) );
					return;
				}

				net->name = name;
				// dict
				m_networks.remove( m_uiCurrentNetworkSelection );
				m_networks.insert( net->name, net );
				// ui
				int idx = networkList->index(networkList->findItem(m_uiCurrentNetworkSelection));
				m_uiCurrentNetworkSelection = net->name;
				networkList->changeItem( net->name, idx ); // changes the selection!!!
				networkList->sort();
			}
		}
	}
}

void IRCNetworkConfigWidget::addNetwork( IRCNetwork *network )
{
	m_networks.insert( network->name, network );
//	slotSaveNetworkConfig();
}

void IRCNetworkConfigWidget::slotMoveServerUp()
{
	IRCHost *selectedHost = m_hosts[hostList->currentText().section(':', 0, 0)];
	IRCNetwork *selectedNetwork = m_networks[networkList->currentText()];

	if( !selectedNetwork || !selectedHost )
		return;

	QValueList<IRCHost*>::iterator pos = selectedNetwork->hosts.find( selectedHost );
	if( pos != selectedNetwork->hosts.begin() )
	{
		QValueList<IRCHost*>::iterator lastPos = pos;
		lastPos--;
		selectedNetwork->hosts.insert( lastPos, selectedHost );
		selectedNetwork->hosts.remove( pos );
	}

	unsigned int currentPos = hostList->currentItem();
	if( currentPos > 0 )
	{
		hostList->removeItem( currentPos );
		kdDebug(14121) << k_funcinfo << selectedHost->host << endl;
		hostList->insertItem( selectedHost->host, --currentPos );
		hostList->setSelected( currentPos, true );
	}
}

void IRCNetworkConfigWidget::slotMoveServerDown()
{
	IRCHost *selectedHost = m_hosts[ hostList->currentText().section(':', 0, 0) ];
	IRCNetwork *selectedNetwork = m_networks[ networkList->currentText() ];

	if( !selectedNetwork || !selectedHost )
		return;

	QValueList<IRCHost*>::iterator pos = selectedNetwork->hosts.find( selectedHost );
	if( *pos != selectedNetwork->hosts.back() )
	{
		QValueList<IRCHost*>::iterator nextPos = pos;
		nextPos++;
		selectedNetwork->hosts.insert( nextPos, selectedHost );
		selectedNetwork->hosts.remove( pos );
	}

	unsigned int currentPos = hostList->currentItem();
	if( currentPos < ( hostList->count() - 1 ) )
	{
		hostList->removeItem( currentPos );
		hostList->insertItem( selectedHost->host, ++currentPos );
		hostList->setSelected( currentPos, true );
	}
}

#include "networkconfigwidget.moc"
