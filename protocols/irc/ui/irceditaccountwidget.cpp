/*
    msneditaccountwidget.cpp - MSN Account Widget

    Copyright (c) 2003 by Olivier Goffart  <ogoffart@tiscalinet.be>

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
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <kdebug.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>

#include "ircaccount.h"
#include "irceditaccountwidget.h"

IRCEditAccountWidget::IRCEditAccountWidget(IRCProtocol *proto, IRCAccount *ident, QWidget *parent, const char * )
				  : IRCEditAccountBase(parent), EditAccountWidget(ident)
{
	mProtocol = proto;

	m_IRCAccount = (IRCAccount *)ident;
	if( m_IRCAccount )
	{
		QString nickName = m_IRCAccount->accountId().section( '@', 0, 0);
		QString serverInfo = m_IRCAccount->accountId().section( '@', 1);

		mNickName->setText( nickName );
		mServer->setText( serverInfo.section(':', 0, 0) );
		mPort->setValue( serverInfo.section(':',1).toUInt() );

		mNickName->setReadOnly(true);
		mServer->setReadOnly(true);

		mUserName->setText( m_IRCAccount->userName() );

		if(m_account->rememberPassword()) mPassword->setText( m_IRCAccount->password() );
		
		QStringList cmds = m_IRCAccount->connectCommands();
		for( QStringList::Iterator i = cmds.begin(); i != cmds.end(); ++i )
			new QListViewItem( commandList, *i );
	}
	
	connect( commandList, SIGNAL( contextMenu( KListView *, QListViewItem *, const QPoint & ) ),
		this, SLOT( slotContextMenu( KListView *, QListViewItem *, const QPoint & ) ) );
		
	connect( addButton, SIGNAL( clicked() ), this, SLOT( slotAddCommand() ) );
}

IRCEditAccountWidget::~IRCEditAccountWidget()
{
}

void IRCEditAccountWidget::slotContextMenu( KListView *, QListViewItem *item, const QPoint &p )
{
	QPopupMenu popup;
	popup.insertItem( i18n("Remove Command"), 1 );
	if( popup.exec( p ) == 1 )
		delete item;
}

void IRCEditAccountWidget::slotAddCommand()
{
	new QListViewItem( commandList, commandEdit->text() );
	commandEdit->clear();
}

KopeteAccount *IRCEditAccountWidget::apply()
{
	QString mAccountId = mNickName->text() + QString::fromLatin1("@") + mServer->text() + QString::fromLatin1(":") + QString::number( mPort->value() );

	if( !m_IRCAccount )
		m_IRCAccount = new IRCAccount( mProtocol, mAccountId );
//	else
//		m_IRCAccount->setAccountId( mAccountId );

	if (mRememberPassword->isChecked()) {
		kdDebug(14120) << k_funcinfo << "Saving password '" << mPassword->text() << "' empty: " << mPassword->text().isEmpty() << " null: " <<  mPassword->text().isNull() << endl;
		m_IRCAccount->setPassword( mPassword->text() );
	}
	
	m_IRCAccount->setUserName( mUserName->text() );
	m_IRCAccount->setAutoLogin( mAutoConnect->isChecked() );
	
	QStringList cmds;
	for( QListViewItem *i = commandList->firstChild(); i; i = i->nextSibling() )
		cmds.append( i->text(0) );
	
	m_IRCAccount->setConnectCommands( cmds );
	
	return m_IRCAccount;
}


bool IRCEditAccountWidget::validateData()
{
	if( mNickName->text().isEmpty() )
		KMessageBox::sorry(this, i18n("<qt>You must enter a nickname.</qt>"), i18n("Kopete"));
	else if( mServer->text().isEmpty() )
		KMessageBox::sorry(this, i18n("<qt>You must enter a server.</qt>"), i18n("Kopete"));
	else
		return true;

	return false;
}

#include "irceditaccountwidget.moc"
