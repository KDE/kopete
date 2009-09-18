/*
    networkconfigwidget.cpp - IRC Network configurator widget.

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

#include "networkconfigwidget.h"
#include "channellist.h"
#include "ircaddcontactpage.h"
#include "ircguiclient.h"
#include "irceditaccountwidget.h"
#include "irctransferhandler.h"

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
#include <QList>

#include <dom/html_element.h>
#include <unistd.h>

using namespace Kopete;

class IRCNetworkConfigWidget::Private
{
public:
	QMap<QString, IRC::Network> m_networks;

	QString m_uiCurrentNetworkSelection;
	int m_uiCurrentHostSelectionIndex;
};

IRCNetworkConfigWidget::IRCNetworkConfigWidget(QWidget *parent, Qt::WindowFlags flags)
	: QDialog(parent, flags),
	  d( new IRCNetworkConfigWidget::Private )
{
//	kDebug(14120) ;
	setupUi( this );

	foreach( const IRC::Network& net, IRC::Networks::self()->networks() )
	{
		d->m_networks.insert( net.name, net );
	}

//	m_host->setValidator( new QRegExpValidator( QString::fromLatin1("^[\\w-\\.]*$"), this ) );
	upButton->setIcon( KIcon( "go-up" )  );
	downButton->setIcon( KIcon( "go-down" ) );

	connect(m_networkList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkConfig()));
	connect(m_hostList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));

	connect(this, SIGNAL(accepted()),
		this, SLOT(slotSaveNetworkConfig()));

	connect(upButton, SIGNAL(clicked()),
		this, SLOT(slotMoveServerUp()));
	connect(downButton, SIGNAL(clicked()),
		this, SLOT(slotMoveServerDown()));

	connect(m_newNetworkButton, SIGNAL(clicked()),
		this, SLOT(slotNewNetwork()));
	connect(m_renameNetworkButton, SIGNAL(clicked()),
		this, SLOT(slotRenameNetwork()));
	connect(m_removeNetworkButton, SIGNAL(clicked()),
		this, SLOT(slotDeleteNetwork()));

	connect(m_removeHost, SIGNAL(clicked()),
		this, SLOT(slotDeleteHost()));
	connect(m_newHost, SIGNAL(clicked()),
		this, SLOT(slotNewHost()));

	connect(port, SIGNAL(valueChanged(int)),
		this, SLOT(slotHostPortChanged(int)));
}

IRCNetworkConfigWidget::~IRCNetworkConfigWidget()
{
}

void IRCNetworkConfigWidget::editNetworks(const QString &networkName)
{
	disconnect(m_networkList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkConfig()));
	disconnect(m_hostList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));

	m_networkList->clear();

	foreach( const IRC::Network & net, d->m_networks)
	{
		m_networkList->insertItem( net.name );
	}

	m_networkList->sort();

	connect(m_networkList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkConfig()));
	connect(m_hostList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));

	if( !networkName.isEmpty() )
		m_networkList->setSelected(m_networkList->findItem(networkName), true);

	//slotUpdateNetworkConfig(); // unnecessary, setSelected emits selectionChanged
}
/*
void IRCNetworkConfigWidget::updateNetworkList()
{
	networkList->clear();
	for(QValueList<IRC::Network>::ConstIterator it = m_networks.begin(); it != m_networks.end(); ++it)
		networkList->insertItem( (*it).name );
	networkList->sort();

//	if (!networkName.isEmpty())
//		networkList->setSelected(networkList->findItem(networkName), true);
}
*/
void IRCNetworkConfigWidget::slotUpdateNetworkConfig()
{
	kDebug(14120)<<"updating network config";
	// update the data structure of the previous selection from the UI
	storeCurrentNetwork();

	// update the UI from the data for the current selection

	if (d->m_networks.contains( m_networkList->currentText() ) )
	{
		IRC::Network net=d->m_networks[ m_networkList->currentText() ];
		m_description->setText( net.description );
		m_hostList->clear();
		d->m_uiCurrentHostSelectionIndex=-1;

		for( QList<IRC::Host>::Iterator it = net.hosts.begin(); it != net.hosts.end(); ++it )
			m_hostList->insertItem( (*it).host + QString::fromLatin1(":") + QString::number((*it).port) );

		// prevent nested event loop crash
		disconnect(m_hostList, SIGNAL(selectionChanged()),
			this, SLOT( slotUpdateNetworkHostConfig() ) );

		m_hostList->setSelected( 0, true );
		slotUpdateNetworkHostConfig();

		connect(m_hostList, SIGNAL(selectionChanged()),
			this, SLOT(slotUpdateNetworkHostConfig()));
	}

	// record the current selection
	d->m_uiCurrentNetworkSelection = m_networkList->currentText();

}

void IRCNetworkConfigWidget::storeCurrentNetwork()
{
	if ( !d->m_uiCurrentNetworkSelection.isEmpty() )
	{
		if ( d->m_networks.contains( d->m_uiCurrentNetworkSelection ) )
		{
			d->m_networks[ d->m_uiCurrentNetworkSelection ].description = m_description->text(); // crash on 2nd dialog show here!
		}
		else
		{
			kDebug( 14120 ) << d->m_uiCurrentNetworkSelection << " was already gone from the cache!";
		}
	}
}

void IRCNetworkConfigWidget::storeCurrentHost()
{
	if ( !d->m_uiCurrentNetworkSelection.isEmpty() && d->m_uiCurrentHostSelectionIndex!=-1  )
	{
		kDebug( 14120 ) <<"storing host id= "<<d->m_uiCurrentHostSelectionIndex;
		IRC::Host &host = d->m_networks[ d->m_uiCurrentNetworkSelection ].hosts[ d->m_uiCurrentHostSelectionIndex ];
		kDebug( 14120 ) <<"old: "<<host.host<<" "<<host.port<<" "<<host.ssl;

		if(!m_host->text().isEmpty())
		{
			host.host = m_host->text();
// 		host.password = password->text();
			host.port = port->text().toInt();
			host.ssl = useSSL->isChecked();
		}

		kDebug( 14120 ) <<"new: "<<host.host<<" "<<host.port<<" "<<host.ssl;
	}
}

void IRCNetworkConfigWidget::slotHostPortChanged( int value )
{
	QString entryText = m_hostList->text( d->m_uiCurrentHostSelectionIndex ) + QString::fromLatin1(":") + QString::number( value );
	// changeItem causes a take() and insert, and we don't want a selectionChanged() signal that sets all this off again.
	disconnect(m_hostList, SIGNAL(selectionChanged()),
		this, SLOT( slotUpdateNetworkHostConfig() ) );

	m_hostList->changeItem(entryText, m_hostList->currentItem());

	connect(m_hostList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));

}

void IRCNetworkConfigWidget::slotUpdateNetworkHostConfig()
{
	storeCurrentHost();

	if (m_hostList->selectedItem())
	{
		d->m_uiCurrentHostSelectionIndex = m_hostList->currentItem();
		int hostIndex=m_hostList->currentItem();
		kDebug(14120)<<"host index= "<< hostIndex;

		if ( hostIndex < d->m_networks[ d->m_uiCurrentNetworkSelection ].hosts.size() )
		{
			IRC::Host host = d->m_networks[ d->m_uiCurrentNetworkSelection ].hosts[ hostIndex ];

			m_host->setText( host.host );
			//password->setText( host.password );
			port->setValue( host.port );
			useSSL->setChecked( host.ssl );

			upButton->setEnabled(m_hostList->currentItem() > 0);
			downButton->setEnabled(m_hostList->currentItem() < (int)(m_hostList->count()-1));
		}
	}
	else
	{
		d->m_uiCurrentHostSelectionIndex = -1;
		disconnect(port, SIGNAL(valueChanged(int)),
			this, SLOT( slotHostPortChanged( int ) ) );
		m_host->clear();
		//password->clear();
		port->setValue( 6667 );
		useSSL->setChecked( false );
		connect(port, SIGNAL(valueChanged(int)),
			this, SLOT( slotHostPortChanged( int ) ) );
	}
}

void IRCNetworkConfigWidget::slotDeleteNetwork()
{
	QString network = m_networkList->currentText();
	if( KMessageBox::warningContinueCancel(
		UI::Global::mainWidget(), i18n("<qt>Are you sure you want to delete the network <b>%1</b>?<br>"
		"Any accounts which use this network will have to be modified.</qt>")
		.arg(network), i18n("Deleting Network"),
		KGuiItem(i18n("&Delete Network"),"edit-delete"), KStandardGuiItem::cancel(), QString::fromLatin1("AskIRCDeleteNetwork") ) == KMessageBox::Continue )
	{
		d->m_networks.remove( network );
		m_networkList->removeItem(m_networkList->currentItem());
		slotUpdateNetworkHostConfig();
	}
}

void IRCNetworkConfigWidget::slotDeleteHost()
{
	QString hostName = m_host->text();
	if ( KMessageBox::warningContinueCancel(
		UI::Global::mainWidget(), i18n("<qt>Are you sure you want to delete the host <b>%1</b>?</qt>")
		.arg(hostName), i18n("Deleting Host"),
		KGuiItem(i18n("&Delete Host"),"edit-delete"), KStandardGuiItem::cancel(), QString::fromLatin1("AskIRCDeleteHost")) == KMessageBox::Continue )
	{
		if ( m_hostList->selectedItem() )
		{
			int hostIndex=m_hostList->currentItem();
			IRC::Host host = d->m_networks[d->m_uiCurrentNetworkSelection].hosts[hostIndex];
			disconnect(m_hostList, SIGNAL(selectionChanged()),
				this, SLOT( slotUpdateNetworkHostConfig() ) );

			m_hostList->removeItem(hostIndex);

			connect(m_hostList, SIGNAL(selectionChanged()),
				this, SLOT(slotUpdateNetworkHostConfig()));

			// remove from network as well
			d->m_networks[ d->m_uiCurrentNetworkSelection ].hosts.removeAt(hostIndex);
		}
	}
}

void IRCNetworkConfigWidget::slotNewNetwork()
{
	kDebug(14120)<<"Creating new network";
	// create a new network struct
	IRC::Network net;
	// give it the name of 'New Network' (incrementing number if needed)
	QString netName = QString::fromLatin1( "New Network" );
	if ( d->m_networks.contains( netName ) )
	{
		int newIdx = 1;
		do {
			netName = QString::fromLatin1( "New Network #%1" ).arg( newIdx++ );
		}
		while ( d->m_networks.contains( netName ) && newIdx < 100 );
		if ( newIdx == 100 ) // pathological case
			return;
	}
	net.name = netName;
	// and add it to the networks dict and list
	d->m_networks.insert(net.name, net);
	m_networkList->insertItem(net.name);
	Q3ListBoxItem * justAdded = m_networkList->findItem(net.name);
	m_networkList->setSelected(justAdded, true);
	m_networkList->setBottomItem(m_networkList->index(justAdded));
}

void IRCNetworkConfigWidget::slotNewHost()
{
	// create a new host
	IRC::Host host;
	// prompt for a name
	bool ok;
	QString name = KInputDialog::getText(
			i18n("New Host"),
			i18n("Enter the hostname of the new server:"),
			QString(), &ok, UI::Global::mainWidget() );
	if ( ok )
	{
		// dupe check
		foreach( const IRC::Host & h, d->m_networks[d->m_uiCurrentNetworkSelection].hosts )
		{
			if ( h.host == name ) //dupe found
				{
				KMessageBox::sorry(this, i18n( "A host already exists with that name" ) );
				return;
			}
		}
		// set defaults on others
		host.host = name;
		host.port = 6667;
		host.ssl = false;
		// add it to the network!
		d->m_networks[ m_networkList->currentText() ].hosts.append( host );
		// add it to the gui
		QString entryText = host.host + QString::fromLatin1(":") + QString::number(host.port);
		m_hostList->insertItem( entryText );
		// select it in the gui
		Q3ListBoxItem * justAdded = m_hostList->findItem( entryText );
		m_hostList->setSelected( justAdded, true );
		//hostList->setBottomItem(hostList->index(justAdded));
	}
}

void IRCNetworkConfigWidget::slotRenameNetwork()
{
	if ( d->m_networks.contains( d->m_uiCurrentNetworkSelection ) )
	{
		IRC::Network net =d-> m_networks[ d->m_uiCurrentNetworkSelection ];
		bool ok;
		// popup up a dialog containing the current name
		QString name = KInputDialog::getText(
				i18n("Rename Network"),
				i18n("Enter the new name for this network:"),
				d->m_uiCurrentNetworkSelection, &ok,
				UI::Global::mainWidget() );
		if ( ok )
		{
			if ( d->m_uiCurrentNetworkSelection != name )
			{
				// dupe check
				if ( d->m_networks.contains( name ) )
				{
					KMessageBox::sorry(this, i18n( "A network already exists with that name" ) );
					return;
				}

				net.name = name;
				kDebug( 14120 )<<"changing name from "<<d->m_uiCurrentNetworkSelection<<" to "<<name;
				// dict
				d->m_networks.remove( d->m_uiCurrentNetworkSelection );
				d->m_networks.insert( net.name, net );

        // ui
				int idx = m_networkList->index(m_networkList->findItem(d->m_uiCurrentNetworkSelection));
				d->m_uiCurrentNetworkSelection = net.name;
				m_networkList->changeItem( net.name, idx ); // changes the selection!!!
				m_networkList->sort();
			}
		}
	}
}

void IRCNetworkConfigWidget::slotMoveServerUp()
{
	IRC::Network& selectedNetwork = d->m_networks[ m_networkList->currentText() ];
	IRC::Host selectedHost = selectedNetwork.hosts[ m_hostList->currentItem() ];

	//if( !selectedNetwork || !selectedHost )
	//	return;

	unsigned int currentPos = m_hostList->currentItem();
	if( currentPos > 0 )
	{
		selectedNetwork.hosts.swap( m_hostList->currentItem(),m_hostList->currentItem()-1 );
		
		disconnect(m_hostList, SIGNAL(selectionChanged()),
			this, SLOT(slotUpdateNetworkHostConfig()));

		m_hostList->removeItem( currentPos );
		kDebug(14120) << selectedHost.host;
		m_hostList->insertItem( selectedHost.host, --currentPos );
		m_hostList->setSelected( currentPos, true );
		d->m_uiCurrentHostSelectionIndex=currentPos;

		connect(m_hostList, SIGNAL(selectionChanged()),
			this, SLOT(slotUpdateNetworkHostConfig()));
	}
}

void IRCNetworkConfigWidget::slotMoveServerDown()
{
	IRC::Network& selectedNetwork = d->m_networks[ m_networkList->currentText() ];
	IRC::Host selectedHost = selectedNetwork.hosts[ m_hostList->currentItem() ];

	//if( !selectedNetwork || !selectedHost )
	//	return;

	unsigned int currentPos = m_hostList->currentItem();
	if( currentPos > 0 )
	{
		selectedNetwork.hosts.swap( m_hostList->currentItem(),m_hostList->currentItem()-1 );
		
		disconnect(m_hostList, SIGNAL(selectionChanged()),
			this, SLOT(slotUpdateNetworkHostConfig()));

		m_hostList->removeItem( currentPos );
		kDebug(14120) << selectedHost.host;
		m_hostList->insertItem( selectedHost.host, --currentPos );
		m_hostList->setSelected( currentPos, true );
		d->m_uiCurrentHostSelectionIndex=currentPos;

		connect(m_hostList, SIGNAL(selectionChanged()),
			this, SLOT(slotUpdateNetworkHostConfig()));
	}
}


void IRCNetworkConfigWidget::slotSaveNetworkConfig()
{
	//HACK: remove the empty entry
	d->m_networks.remove(QString());

	IRC::NetworkList networks=d->m_networks.values();	//Convert from map to list
	
	IRC::Networks::self()->setNetworks(networks);
	IRC::Networks::self()->slotSaveNetworkConfig();
}


#include "networkconfigwidget.moc"
