/*
   irceditaccountwidget.h - IRC Account Widget

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



#ifndef IRCEDITACCOUNTWIDEGET_H
#define IRCEDITACCOUNTWIDEGET_H

#include "editaccountwidget.h"
#include "irceditaccount.h"

class IRCProtocol;
class IRCAccount;
class KListView;
class QListViewItem;

class IRCEditAccountWidget : public IRCEditAccountBase, public KopeteEditAccountWidget
{
	Q_OBJECT

	public:
		IRCEditAccountWidget(IRCProtocol *proto, IRCAccount *, QWidget *parent=0, const char *name=0);
		~IRCEditAccountWidget();

		IRCAccount *account();
		virtual bool validateData();
		virtual Kopete::Account *apply();

	private slots:
		void slotCommandContextMenu( KListView*, QListViewItem*, const QPoint & );
		void slotCtcpContextMenu( KListView*, QListViewItem*, const QPoint & );
		void slotAddCommand();
		void slotAddCtcp();
		void slotEditNetworks();
		void slotUpdateNetworks( const QString & );
		void slotUpdateNetworkDescription( const QString & );

	private:
		void readNetworks();
		QString generateAccountId( const QString &network );

		IRCProtocol *mProtocol;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

