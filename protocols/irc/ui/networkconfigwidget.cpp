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

	upButton->setIcon( KIcon( "go-up" )  );
	downButton->setIcon( KIcon( "go-down" ) );

	connect(m_networkList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotUpdateNetworkConfig()));
	connect(m_hostList, SIGNAL(itemSelectionChanged()),
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
	
	connect(useSSL, SIGNAL(stateChanged(int)),
		this, SLOT(slotUseSSLChanged(int)));
}

IRCNetworkConfigWidget::~IRCNetworkConfigWidget()
{
}

void IRCNetworkConfigWidget::editNetworks(const QString &networkName)
{
	disconnect(m_networkList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotUpdateNetworkConfig()));
	disconnect(m_hostList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));

	m_networkList->clear();

	foreach( const IRC::Network & net, d->m_networks)
	{
		m_networkList->addItem( net.name );
	}

	m_networkList->sortItems();

	connect(m_networkList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotUpdateNetworkConfig()));
	connect(m_hostList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));

	QList<QListWidgetItem *> t_items = m_networkList->findItems(networkName, Qt::MatchStartsWith);
	if(!networkName.isEmpty() && (t_items.count() > 0))
		m_networkList->setCurrentItem(t_items.at(0));
}

void IRCNetworkConfigWidget::slotUpdateNetworkConfig()
{
	//kDebug(14120)<<"updating network config";
	// update the data structure of the previous selection from the UI
	storeCurrentNetwork();

	// record the current selection
	d->m_uiCurrentNetworkSelection = m_networkList->currentItem()->text();

	// update the UI from the data for the current selection
	if (d->m_networks.contains( m_networkList->currentItem()->text() ) )
	{
		IRC::Network net = d->m_networks[ m_networkList->currentItem()->text() ];
		m_description->setText( net.description );
		m_hostList->clear();
		d->m_uiCurrentHostSelectionIndex = -1;

		for( QList<IRC::Host>::Iterator it = net.hosts.begin(); it != net.hosts.end(); ++it )
			m_hostList->addItem( (*it).host + QString::fromLatin1(":") + QString::number((*it).port) );

		// prevent nested event loop crash
		disconnect(m_hostList, SIGNAL(itemSelectionChanged()),
			this, SLOT( slotUpdateNetworkHostConfig() ) );

		m_hostList->setCurrentRow( 0 );
		slotUpdateNetworkHostConfig();
		d->m_uiCurrentHostSelectionIndex = 0;

		connect(m_hostList, SIGNAL(itemSelectionChanged()),
			this, SLOT(slotUpdateNetworkHostConfig()));
	}

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

void IRCNetworkConfigWidget::slotHostPortChanged( int value )
{
	QString entryText = m_hostList->currentItem()->text();
	entryText = entryText.left(entryText.lastIndexOf(':'));  //Cut away the old port
	entryText += QString::fromLatin1(":") + QString::number( value );
	
	disconnect(m_hostList, SIGNAL(itemSelectionChanged()),
		this, SLOT( slotUpdateNetworkHostConfig() ) );

	m_hostList->currentItem()->setText(entryText);

	connect(m_hostList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));
	
	d->m_networks[d->m_uiCurrentNetworkSelection].hosts[d->m_uiCurrentHostSelectionIndex].port = value;
}

void IRCNetworkConfigWidget::slotUseSSLChanged(int state)
{
	d->m_networks[d->m_uiCurrentNetworkSelection].hosts[d->m_uiCurrentHostSelectionIndex].ssl = (state == Qt::Checked);
}

void IRCNetworkConfigWidget::slotUpdateNetworkHostConfig()
{
	if (m_hostList->currentRow() != -1)
	{
		int hostIndex = m_hostList->currentRow();
		d->m_uiCurrentHostSelectionIndex = hostIndex;

		if ( hostIndex < d->m_networks[ d->m_uiCurrentNetworkSelection ].hosts.size() )
		{
			IRC::Host host = d->m_networks[ d->m_uiCurrentNetworkSelection ].hosts[ hostIndex ];

			m_host->setText( host.host );
			//password->setText( host.password );
			port->setValue( host.port );
			useSSL->setChecked( host.ssl );

			upButton->setEnabled(m_hostList->currentRow() > 0);
			downButton->setEnabled(m_hostList->currentRow() < (int)(m_hostList->count() - 1));
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
	QString network = m_networkList->currentItem()->text();
	if( KMessageBox::warningContinueCancel(
		UI::Global::mainWidget(), i18n("<qt>Are you sure you want to delete the network <b>%1</b>?<br>"
		"Any account which use this network will have to be modified.</qt>")
		.arg(network), i18n("Deleting Network"),
		KGuiItem(i18n("&Delete Network"),"edit-delete"), KStandardGuiItem::cancel(), QString::fromLatin1("AskIRCDeleteNetwork") ) == KMessageBox::Continue )
	{
		d->m_networks.remove( network );
		m_networkList->removeItemWidget(m_networkList->currentItem());
		slotUpdateNetworkHostConfig();
	}
}

void IRCNetworkConfigWidget::slotDeleteHost()
{
	QString hostName = m_host->text();

	if (KMessageBox::warningContinueCancel(
	    UI::Global::mainWidget(),
	    i18n("<qt>Are you sure you want to delete the host <b>%1</b>?</qt>").arg(hostName),
	    i18n("Deleting Host"),
	    KGuiItem(i18n("&Delete Host"),"edit-delete"),
	    KStandardGuiItem::cancel(),
	    QString::fromLatin1("AskIRCDeleteHost")) != KMessageBox::Continue)
		return;

	if ( !m_hostList->selectedItems().count() )
		return;

	int hostIndex=m_hostList->currentRow();
	IRC::Host host = d->m_networks[d->m_uiCurrentNetworkSelection].hosts[hostIndex];
	disconnect(m_hostList, SIGNAL(itemSelectionChanged()),
		this, SLOT( slotUpdateNetworkHostConfig() ) );

	m_hostList->removeItemWidget(m_hostList->item(hostIndex));

	connect(m_hostList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));

	// remove from network as well
	d->m_networks[ d->m_uiCurrentNetworkSelection ].hosts.removeAt(hostIndex);
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
		
		do
		{
			netName = QString::fromLatin1( "New Network #%1" ).arg( newIdx++ );
		}
		while ( d->m_networks.contains( netName ) && newIdx < 100 );
		
		if ( newIdx == 100 ) // pathological case
			return;
	}
	net.name = netName;
	// and add it to the networks dict and list
	d->m_networks.insert(net.name, net);
	m_networkList->addItem(net.name);
	//Q3ListBoxItem * justAdded = m_networkList->findItem(net.name);
	QListWidgetItem* justAdded = m_networkList->findItems(net.name, Qt::MatchStartsWith).at(0);
	m_networkList->setCurrentItem(justAdded);
	//m_networkList->setBottomItem(m_networkList->index(justAdded));
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
	if (!ok)
		return;
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
	d->m_networks[ m_networkList->currentItem()->text() ].hosts.append( host );
	// add it to the gui
	QString entryText = host.host + QString::fromLatin1(":") + QString::number(host.port);
	m_hostList->addItem( entryText );
	// select it in the gui
	QListWidgetItem* justAdded = m_hostList->findItems(entryText, Qt::MatchStartsWith).at(0);
	m_hostList->setCurrentItem(justAdded);
}

void IRCNetworkConfigWidget::slotRenameNetwork()
{
	if ( !d->m_networks.contains( d->m_uiCurrentNetworkSelection ) )
		return;
	
	IRC::Network net = d-> m_networks[ d->m_uiCurrentNetworkSelection ];
	bool ok;
	
	// popup up a dialog containing the current name
	QString name = KInputDialog::getText(
			i18n("Rename Network"),
			i18n("Enter the new name for this network:"),
			d->m_uiCurrentNetworkSelection, &ok,
			UI::Global::mainWidget() );
	if (!ok || d->m_uiCurrentNetworkSelection == name)
		return;
			
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
	QList<QListWidgetItem*> found = m_networkList->findItems(d->m_uiCurrentNetworkSelection, Qt::MatchStartsWith);
	d->m_uiCurrentNetworkSelection = net.name;
	found.at(0)->setText(net.name);
	m_networkList->sortItems();
	
	m_networkList->scrollToItem(found.at(0));
}

void IRCNetworkConfigWidget::slotMoveServerUp()
{
	IRC::Network& selectedNetwork = d->m_networks[ m_networkList->currentItem()->text() ];
	//IRC::Host selectedHost = selectedNetwork.hosts[ m_hostList->currentRow() ];

	//if( !selectedNetwork || !selectedHost )
	//	return;

	int currentPos = m_hostList->currentRow();
	if( currentPos < 0 )
		return;
	
	disconnect(m_hostList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));

	selectedNetwork.hosts.swap(currentPos - 1, currentPos);

	QString first = m_hostList->item(currentPos - 1)->text();
	QString second = m_hostList->item(currentPos)->text();
	m_hostList->item(currentPos - 1)->setText(second);
	m_hostList->item(currentPos)->setText(first);
	
	/*
	m_hostList->removeItemWidget( m_hostList->item(currentPos) );
	kDebug(14120) << selectedHost.host;
	m_hostList->insertItem( --currentPos, selectedHost.host);
	m_hostList->setCurrentRow(currentPos);
	d->m_uiCurrentHostSelectionIndex=currentPos;
	*/
	connect(m_hostList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));
	
	m_hostList->setCurrentRow(currentPos - 1);
}

void IRCNetworkConfigWidget::slotMoveServerDown()
{
	IRC::Network& selectedNetwork = d->m_networks[ m_networkList->currentItem()->text() ];
	//IRC::Host selectedHost = selectedNetwork.hosts[ m_hostList->currentRow() ];

	//if( !selectedNetwork || !selectedHost )
	//	return;

	int currentPos = m_hostList->currentRow();
	if( currentPos < 0 )
		return;

	disconnect(m_hostList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));

	selectedNetwork.hosts.swap(currentPos, currentPos + 1);

	QString first = m_hostList->item(currentPos)->text();
	QString second = m_hostList->item(currentPos + 1)->text();
	m_hostList->item(currentPos)->setText(second);
	m_hostList->item(currentPos + 1)->setText(first);

	/*
	m_hostList->removeItemWidget( m_hostList->item(currentPos) );
	kDebug(14120) << selectedHost.host;
	m_hostList->insertItem( --currentPos, selectedHost.host);
	m_hostList->setCurrentRow(currentPos);
	d->m_uiCurrentHostSelectionIndex=currentPos;
	*/
	connect(m_hostList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotUpdateNetworkHostConfig()));
	
	m_hostList->setCurrentRow(currentPos + 1);
}



void IRCNetworkConfigWidget::slotSaveNetworkConfig()
{
	storeCurrentNetwork();

	//HACK: remove the empty entry
	d->m_networks.remove(QString());

	IRC::NetworkList networks=d->m_networks.values();	//Convert from map to list
	
	IRC::Networks::self()->setNetworks(networks);
	IRC::Networks::self()->slotSaveNetworkConfig();
}

QString IRCNetworkConfigWidget::selectedNetwork() const
{
	return d->m_uiCurrentNetworkSelection;
}

#include "networkconfigwidget.moc"
