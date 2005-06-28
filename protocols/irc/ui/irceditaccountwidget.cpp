/*
    irceditaccountwidget.cpp - IRC Account Widget

    Copyright (c) 2003 by Olivier Goffart  <ogoffart @ kde.org>
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

#include "irceditaccountwidget.h"

#include "ircaccount.h"
#include "irccontact.h"
#include "ircprotocol.h"

#include "kircengine.h"

#include "kopetepasswordwidget.h"

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
#include <qtextcodec.h>

IRCEditAccountWidget::IRCEditAccountWidget(IRCAccount *ident, QWidget *parent, const char * )
	  : IRCEditAccountBase(parent), KopeteEditAccountWidget(ident)
{
	int currentCodec = 4;

	if( account() )
	{
		QString nickName = account()->mySelf()->nickName();
		QString serverInfo = account()->accountId();

		mNickName->setText( nickName );
//		mAltNickname->setText( account()->altNick() );
		mUserName->setText( account()->userName() );
		m_realNameLineEdit->setText( account()->realName() );

		partMessage->setText( account()->defaultPart() );
		quitMessage->setText( account()->defaultQuit() );
		if( account()->codec() )
			currentCodec = account()->codec()->mibEnum();

		mPasswordWidget->load ( &account()->password() );

		preferSSL->setChecked(account()->configGroup()->readBoolEntry("PreferSSL"));
		autoShowServerWindow->setChecked( account()->configGroup()->readBoolEntry("AutoShowServerWindow") );
		autoConnect->setChecked( static_cast<Kopete::Account*>(account())->excludeConnect() );

		KConfigGroup *config = account()->configGroup();

		QStringList cmds = account()->connectCommands();
		for( QStringList::Iterator i = cmds.begin(); i != cmds.end(); ++i )
			new QListViewItem( commandList, *i );

		const QMap< QString, QString > replies = account()->customCtcpReplies();
		for( QMap< QString, QString >::ConstIterator it = replies.begin(); it != replies.end(); ++it )
			new QListViewItem( ctcpList, it.key(), it.data() );
	}

	mUserName->setValidator( new QRegExpValidator( QString::fromLatin1("^[^\\s]*$"), mUserName ) );
	mNickName->setValidator( new QRegExpValidator( QString::fromLatin1("^[^#+&][^\\s]*$"), mNickName ) );
	mAltNickname->setValidator( new QRegExpValidator( QString::fromLatin1("^[^#+&][^\\s]*$"), mAltNickname ) );

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

	connect( IRCProtocol::self(), SIGNAL( networkConfigUpdated( const QString & ) ),
		this, SLOT( slotUpdateNetworks( const QString & ) ) );

	slotUpdateNetworks( QString::null );
}

IRCEditAccountWidget::~IRCEditAccountWidget()
{
}

IRCAccount *IRCEditAccountWidget::account ()
{
	return dynamic_cast<IRCAccount *>(KopeteEditAccountWidget::account () );
}

void IRCEditAccountWidget::slotUpdateNetworks( const QString & selectedNetwork )
{
	network->clear();

	uint i = 0;
	QStringList keys;
	QValueList<IRCNetwork> networks = IRCNetworkList::self()->networks();

	for(QValueList<IRCNetwork>::Iterator it = networks.begin(); it != networks.end(); ++it )
		keys.append((*it).name);

	keys.sort();
/*
	QStringList::Iterator end = keys.end();
	for( QStringList::Iterator it = keys.begin(); it != end; ++it )
	{
		IRCNetwork * current = IRCProtocol::self()->networks()[*it];
		network->insertItem( current->name );
		if ( ( account() && account()->networkName() == current->name ) || current->name == selectedNetwork )
		{
			network->setCurrentItem( i );
			description->setText( current->description );
		}
		++i;
	}
*/
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
	while( config->hasGroup( QString("Account_%1_%2").arg( IRCProtocol::self()->pluginId() ).arg( nextId ) ) )
	{
		nextId = QString::fromLatin1("%1_%2").arg( network ).arg( ++accountNumber );
	}
	kdDebug( 14120 ) << k_funcinfo << " ID IS: " << nextId << endl;
	return nextId;
}

Kopete::Account *IRCEditAccountWidget::apply()
{
	QString nickName = mNickName->text();
	QString networkName = network->currentText();

	if( !account() )
	{
		setAccount( new IRCAccount( generateAccountId(networkName), QString::null, networkName, nickName ) );

	}
	else
	{
		account()->setNickName( nickName );
//		account()->setNetwork( networkName );
	}

	mPasswordWidget->save( &account()->password() );

//	account()->setAltNick( mAltNickname->text() );
	account()->setUserName( mUserName->text() );
	account()->setRealName( m_realNameLineEdit->text() );
	account()->setDefaultPart( partMessage->text() );
	account()->setDefaultQuit( quitMessage->text() );
	account()->setAutoShowServerWindow( autoShowServerWindow->isChecked() );
	account()->setExcludeConnect( autoConnect->isChecked() );

	account()->configGroup()->writeEntry("PreferSSL", preferSSL->isChecked());

	QStringList cmds;
	for( QListViewItem *i = commandList->firstChild(); i; i = i->nextSibling() )
		cmds.append( i->text(0) );

	QMap< QString, QString > replies;
	for( QListViewItem *i = ctcpList->firstChild(); i; i = i->nextSibling() )
		replies[ i->text(0) ] = i->text(1);

	account()->setCustomCtcpReplies( replies );
	account()->setConnectCommands( cmds );

	KCharsets *c = KGlobal::charsets();
	account()->setCodec( c->codecForName( c->encodingForName( charset->currentText() ) ) );

	return account();
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
