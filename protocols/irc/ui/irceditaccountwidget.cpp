/*
    irceditaccountwidget.cpp - IRC Account Widget

    Copyright (c) 2003      by Olivier Goffart  <ogoffart@kde.org>
    Copyright (c) 2003      by Jason Keirstead  <jason@keirstead.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "irceditaccountwidget.moc"

#include "ircaccount.h"
#include "irccontact.h"
#include "ircprotocol.h"

#include "kopetepasswordwidget.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kcharsets.h>

#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qvalidator.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qtextcodec.h>

#include <algorithm>
#include <kconfiggroup.h>

IRCEditAccountWidget::IRCEditAccountWidget(IRCAccount *ident, QWidget *parent)
	: QWidget(parent), KopeteEditAccountWidget(ident)
{
	setupUi(this);

	int currentCodec = 4;

	if( account() )
	{
		QString nickName = account()->nickName();
		QString serverInfo = account()->accountId();

		nickNames->setText( nickName );
		userName->setText( account()->userName() );
		realName->setText( account()->realName() );

		partMessage->setText( account()->partMessage() );
		quitMessage->setText( account()->quitMessage() );
		if( account()->codec() )
			currentCodec = account()->codec()->mibEnum();

//		mPasswordWidget->load ( &account()->password() );

		preferSSL->setChecked(account()->configGroup()->readEntry("PreferSSL",false));
		autoShowServerWindow->setChecked( account()->configGroup()->readEntry("AutoShowServerWindow", false) );
		autoConnect->setChecked( static_cast<Kopete::Account*>(account())->excludeConnect() );

		//KConfigGroup *config = account()->configGroup();
/*
		QStringList cmds = account()->connectCommands();
		for( QStringList::Iterator i = cmds.begin(); i != cmds.end(); ++i )
			new QListViewItem( commandList, *i );

		const QMap< QString, QString > replies = account()->customCtcpReplies();
		for( QMap< QString, QString >::ConstIterator it = replies.begin(); it != replies.end(); ++it )
			new QListViewItem( ctcpList, it.key(), it.data() );
*/
	}

//	mUserName->setValidator( new QRegExpValidator( QString::fromLatin1("^[^\\s]*$"), mUserName ) );
//	mNickName->setValidator( new QRegExpValidator( QString::fromLatin1("^[^#+&][^\\s]*$"), mNickName ) );
//	mAltNickname->setValidator( new QRegExpValidator( QString::fromLatin1("^[^#+&][^\\s]*$"), mAltNickname ) );

	KCharsets *c = KGlobal::charsets();
	charset->addItems( c->availableEncodingNames() );

	for( int i = 0; i < charset->count(); ++i )
	{
		// codecForName can return NULL
		if( c->codecForName( charset->itemText(i) )->mibEnum() == currentCodec )
		{
			charset->setCurrentIndex( i );
			break;
		}
	}
/*
	connect( commandList, SIGNAL( contextMenu( K3ListView *, QListViewItem *, const QPoint & ) ),
		this, SLOT( slotCommandContextMenu( K3ListView *, QListViewItem *, const QPoint & ) ) );

	connect( ctcpList, SIGNAL( contextMenu( K3ListView *, QListViewItem *, const QPoint & ) ),
		this, SLOT( slotCtcpContextMenu( K3ListView *, QListViewItem *, const QPoint & ) ) );

*/
	connect( addButton, SIGNAL( clicked() ), this, SLOT( slotAddCommand() ) );
	connect( editButton, SIGNAL( clicked() ), this, SLOT(slotEditNetworks() ) );
	connect( addReply, SIGNAL( clicked() ), this, SLOT( slotAddCtcp() ) );


        connect( network, SIGNAL( activated( const QString & ) ),
		this, SLOT( slotUpdateNetworkDescription( const QString &) ) );
//TODO: signal doesn't exist anymore
#if 0
	connect( IRCProtocol::self(), SIGNAL( networkConfigUpdated( const QString & ) ),
		this, SLOT( slotUpdateNetworks( const QString & ) ) );
#endif
	slotUpdateNetworks( QString() );
}

IRCEditAccountWidget::~IRCEditAccountWidget()
{
}

IRCAccount *IRCEditAccountWidget::account ()
{
	return dynamic_cast<IRCAccount *>(KopeteEditAccountWidget::account () );
}

struct NetNameComparator {
	bool operator()(const IRC::Network& a, const IRC::Network& b) const {
		return a.name < b.name;
	}
};

void IRCEditAccountWidget::slotUpdateNetworks( const QString & selectedNetwork )
{

	network->clear();
	kDebug()<<"updating networks. selected="<<selectedNetwork<<endl;

	IRC::NetworkList networks = IRC::Networks::self()->networks();
	std::sort(networks.begin(), networks.end(), NetNameComparator());

	kDebug()<<"got "<<networks.size()<<" networks"<<endl;
	uint i = 0;
	foreach(const IRC::Network& net, networks) {
		network->addItem(net.name);

		if ((account() && account()->networkName() == net.name)
				|| net.name == selectedNetwork)
		{
			network->setCurrentIndex( i );
		}
		++i;
	}
}

void IRCEditAccountWidget::slotEditNetworks()
{
	IRCProtocol::self()->editNetworks(network->currentText());
}

void IRCEditAccountWidget::slotUpdateNetworkDescription( const QString &network )
{
//	description->setText(
//		IRCProtocol::self()->networks()[ network ]->description
//	);
}

void IRCEditAccountWidget::slotCommandContextMenu( K3ListView *, Q3ListViewItem *item, const QPoint &p )
{
/*
	QPopupMenu popup;
	popup.insertItem( i18n("Remove Command"), 1 );
	if( popup.exec( p ) == 1 )
		delete item;
*/
}

void IRCEditAccountWidget::slotCtcpContextMenu( K3ListView *, Q3ListViewItem *item, const QPoint &p )
{
/*
	QPopupMenu popup;
	popup.insertItem( i18n("Remove CTCP Reply"), 1 );
	if( popup.exec( p ) == 1 )
		delete item;
*/
}

void IRCEditAccountWidget::slotAddCommand()
{
/*
	if ( !commandEdit->text().isEmpty() )
	{
		new QListViewItem( commandList, commandEdit->text() );
		commandEdit->clear();
	}
*/
}

void IRCEditAccountWidget::slotAddCtcp()
{
/*
	if (  !newCTCP->text().isEmpty() && !newReply->text().isEmpty() )
	{
		new QListViewItem( ctcpList, newCTCP->text(), newReply->text() );
		newCTCP->clear();
		newReply->clear();
	}
*/
}

QString IRCEditAccountWidget::generateAccountId( const QString &network )
{
	KSharedConfig::Ptr config = KGlobal::config();
	QString nextId = network;

	uint accountNumber = 1;
	while( config->hasGroup( QString("Account_%1_%2").arg( IRCProtocol::self()->pluginId() ).arg( nextId ) ) )
	{
		nextId = QString::fromLatin1("%1_%2").arg( network ).arg( ++accountNumber );
	}
	kDebug( 14120 ) << " ID IS: " << nextId;
	return nextId;
}

Kopete::Account *IRCEditAccountWidget::apply()
{
	QString nickName = nickNames->text();
	QString networkName = network->currentText();

	if( !account() )
	{
		setAccount( new IRCAccount( generateAccountId(networkName), QString(), networkName, nickName ) );
	}
	else
	{
		account()->setNickName( nickName );
		account()->setNetworkByName( networkName );
	}

	//mPasswordWidget->save( &account()->password() );

	account()->setNickName( nickNames->text() );
	account()->setUserName( userName->text() );
	account()->setRealName( realName->text() );
	account()->setPartMessage( partMessage->text() );
	account()->setQuitMessage( quitMessage->text() );
	account()->setAutoShowServerWindow( autoShowServerWindow->isChecked() );
	account()->setExcludeConnect( autoConnect->isChecked() );

	account()->configGroup()->writeEntry("PreferSSL", preferSSL->isChecked());

	/*
	QStringList cmds;
	for( QListViewItem *i = commandList->firstChild(); i; i = i->nextSibling() )
		cmds.append( i->text(0) );

	QMap< QString, QString > replies;
	for( QListViewItem *i = ctcpList->firstChild(); i; i = i->nextSibling() )
		replies[ i->text(0) ] = i->text(1);

	account()->setCustomCtcpReplies( replies );
	account()->setConnectCommands( cmds );
	*/

	KCharsets *c = KGlobal::charsets();
	account()->setCodec( c->codecForName( c->encodingForName( charset->currentText() ) ) );

	return account();
}

bool IRCEditAccountWidget::validateData()
{
	if( nickNames->text().isEmpty() )
		KMessageBox::sorry(this, i18n("<qt>You must enter a nickname.</qt>"), i18n("Kopete"));
	else
		return true;

	return false;
}

