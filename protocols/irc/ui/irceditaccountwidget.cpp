/*
    irceditaccountwidget.cpp - IRC Account Widget

    Copyright (c) 2003 by Olivier Goffart  <ogoffart@tiscalinet.be>
    Copyright (c) 2003 by Jason Keirstead  <jason@keirstead.org>

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
#include <kmessagebox.h>
#include <klocale.h>
#include <klistview.h>
#include <kdebug.h>
#include <kextsock.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kcharsets.h>

#include <qlabel.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qconnection.h>
#include <qvalidator.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qlineedit.h>

#include "kirc.h"
#include "ircaccount.h"
#include "ircusercontact.h"
#include "ircprotocol.h"
#include "irceditaccountwidget.h"

IRCEditAccountWidget::IRCEditAccountWidget(IRCProtocol *proto, IRCAccount *ident, QWidget *parent, const char * )
				  : IRCEditAccountBase(parent), KopeteEditAccountWidget(ident)
{
	mProtocol = proto;

	m_IRCAccount = (IRCAccount *)ident;
	int currentCodec = 4;

	if( m_IRCAccount )
	{
		QString nickName = m_IRCAccount->mySelf()->nickName();
		QString serverInfo = m_IRCAccount->accountId();

		mNickName->setText( nickName );

		mUserName->setText( m_IRCAccount->userName() );
		mAltNickname->setText( m_IRCAccount->altNick() );
		partMessage->setText( m_IRCAccount->defaultPart() );
		quitMessage->setText( m_IRCAccount->defaultQuit() );
		if( m_IRCAccount->codec() )
			currentCodec = m_IRCAccount->codec()->mibEnum();

// 		mPass->load ( &m_IRCAccount->password() );

		preferSSL->setChecked(
			account()->pluginData (m_protocol, "PreferSSL") == QString::fromLatin1("true") );

		//if(account()->rememberPassword()) mPassword->setText( m_IRCAccount->password() );

		QStringList cmds = m_IRCAccount->connectCommands();
		for( QStringList::Iterator i = cmds.begin(); i != cmds.end(); ++i )
			new QListViewItem( commandList, *i );

		const QMap< QString, QString > replies = m_IRCAccount->customCtcpReplies();
		for( QMap< QString, QString >::ConstIterator it = replies.begin(); it != replies.end(); ++it )
			new QListViewItem( ctcpList, it.key(), it.data() );
	}

	mUserName->setValidator( new QRegExpValidator( QString::fromLatin1("^[^\\s]*$"), mUserName ) );
	mNickName->setValidator( new QRegExpValidator( QString::fromLatin1("^[^#+&][^\\s]*$"), mUserName ) );
	mAltNickname->setValidator( new QRegExpValidator( QString::fromLatin1("^[^#+&][^\\s]*$"), mUserName ) );

	KCharsets *c = KGlobal::charsets();
	charset->insertStringList( c->availableEncodingNames() );

	for( int i = 0; i < charset->count(); ++i )
	{
		if( c->codecForName( charset->text(i) )->mibEnum() == currentCodec )
		{
			charset->setCurrentItem( i );
			break;
		}
	}

	connect( commandList, SIGNAL( contextMenu( KListView *, QListViewItem *, const QPoint & ) ),
		this, SLOT( slotCommandContextMenu( KListView *, QListViewItem *, const QPoint & ) ) );

	connect( ctcpList, SIGNAL( contextMenu( KListView *, QListViewItem *, const QPoint & ) ),
		this, SLOT( slotCtcpContextMenu( KListView *, QListViewItem *, const QPoint & ) ) );

	connect( addButton, SIGNAL( clicked() ), this, SLOT( slotAddCommand() ) );
	connect( editButton, SIGNAL( clicked() ), this, SLOT(slotEditNetworks() ) );
	connect( addReply, SIGNAL( clicked() ), this, SLOT( slotAddCtcp() ) );

	connect( network, SIGNAL( activated( const QString & ) ),
		this, SLOT( slotUpdateNetworkDescription( const QString &) ) );

	connect( IRCProtocol::protocol(), SIGNAL( networkConfigUpdated() ),
		this, SLOT( slotUpdateNetworks() ) );

	slotUpdateNetworks();
}

IRCEditAccountWidget::~IRCEditAccountWidget()
{
}

void IRCEditAccountWidget::slotUpdateNetworks()
{
	network->clear();

	uint i = 0;
	QStringList keys;
	for( QDictIterator<IRCNetwork> it( IRCProtocol::protocol()->networks() ); it.current(); ++it )
		keys.append( it.currentKey() );

	keys.sort();

	QStringList::Iterator end = keys.end();
	for( QStringList::Iterator it = keys.begin(); it != end; ++it )
	{
		IRCNetwork * current = IRCProtocol::protocol()->networks()[*it];
		network->insertItem( current->name );
		if( m_IRCAccount && m_IRCAccount->networkName() == current->name )
		{
			network->setCurrentItem( i );
			description->setText( current->description );
		}
		++i;
	}
}

void IRCEditAccountWidget::slotEditNetworks()
{
	IRCProtocol::protocol()->editNetworks( network->currentText() );
}

void IRCEditAccountWidget::slotUpdateNetworkDescription( const QString &network )
{
	description->setText(
		IRCProtocol::protocol()->networks()[ network ]->description
	);
}

void IRCEditAccountWidget::slotCommandContextMenu( KListView *, QListViewItem *item, const QPoint &p )
{
	QPopupMenu popup;
	popup.insertItem( i18n("Remove Command"), 1 );
	if( popup.exec( p ) == 1 )
		delete item;
}

void IRCEditAccountWidget::slotCtcpContextMenu( KListView *, QListViewItem *item, const QPoint &p )
{
	QPopupMenu popup;
	popup.insertItem( i18n("Remove CTCP Reply"), 1 );
	if( popup.exec( p ) == 1 )
		delete item;
}

void IRCEditAccountWidget::slotAddCommand()
{
    if ( !commandEdit->text().isEmpty() )
    {
	new QListViewItem( commandList, commandEdit->text() );
	commandEdit->clear();
    }
}

void IRCEditAccountWidget::slotAddCtcp()
{
    if (  !newCTCP->text().isEmpty() && !newReply->text().isEmpty() )
    {
	new QListViewItem( ctcpList, newCTCP->text(), newReply->text() );
	newCTCP->clear();
	newReply->clear();
    }
}

QString IRCEditAccountWidget::generateAccountId( const QString &network )
{
	KConfig *config = KGlobal::config();
	QString nextId = network;

	uint accountNumber = 1;
	while( config->hasGroup( nextId ) )
	{
		nextId = QString::fromLatin1("%1_%2").arg(network).arg(++accountNumber);
		accountNumber++;
	}

	return nextId;
}

KopeteAccount *IRCEditAccountWidget::apply()
{
	QString nickName = mNickName->text();
	QString networkName = network->currentText();

	if( !m_IRCAccount )
	{
		m_IRCAccount = new IRCAccount( mProtocol, generateAccountId(networkName) );

		m_IRCAccount->setNetwork( networkName );

		m_IRCAccount->loaded();
	}

//	mPass->save( &m_IRCAccount->password() );

	m_IRCAccount->setNickName( nickName );
	m_IRCAccount->setUserName( mUserName->text() );
	m_IRCAccount->setAltNick( mAltNickname->text() );
	m_IRCAccount->setDefaultPart( partMessage->text() );
	m_IRCAccount->setDefaultQuit( quitMessage->text() );
	// Doesn't work
	//m_IRCAccount->setAutoLogin( autoConnect->isChecked() );
	// what about?:
	// password - No time to sort out the password handling or move to KopetePasswordedAccount before 3.3 
	// remember password cb - disabling the UI for this.
	
	if ( preferSSL->isChecked () )
		account()->setPluginData (m_protocol, "PreferSSL", "true");
	else
		account()->setPluginData (m_protocol, "PreferSSL", "false");
	
	QStringList cmds;
	for( QListViewItem *i = commandList->firstChild(); i; i = i->nextSibling() )
		cmds.append( i->text(0) );

	QMap< QString, QString > replies;
	for( QListViewItem *i = ctcpList->firstChild(); i; i = i->nextSibling() )
		replies[ i->text(0) ] = i->text(1);

	m_IRCAccount->setCustomCtcpReplies( replies );
	m_IRCAccount->setConnectCommands( cmds );

	KCharsets *c = KGlobal::charsets();
	m_IRCAccount->setCodec( c->codecForName( c->encodingForName( charset->currentText() ) ) );

	return m_IRCAccount;
}


bool IRCEditAccountWidget::validateData()
{
	if( mNickName->text().isEmpty() )
		KMessageBox::sorry(this, i18n("<qt>You must enter a nickname.</qt>"), i18n("Kopete"));
	else
		return true;

	return false;
}

#include "irceditaccountwidget.moc"
