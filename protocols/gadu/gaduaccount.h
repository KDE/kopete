// gaduaccount.h
//
// Copyright (C)  2003  Zack Rusin <zack@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
#ifndef GADUACCOUNT_H
#define GADUACCOUNT_H

#include "kopeteaccount.h"
#include "gaduprotocol.h"

class GaduAccount : public KopeteAccount
{
  Q_OBJECT
public:
	typedef QMap< uin_t, GaduContact* > ContactsMap;
	GaduAccount( KopeteProtocol* parent, const QString& accountID,
							 const char* name=0L );
	//{
	void setAway( bool isAway, const QString& awayMessage = QString::null );
	KopeteContact* myself() const;
	KActionMenu*   actionMenu();
	//}
public slots:
  //{
  void connect();
	void disconnect();
  //}

	void slotGoOnline();
	void slotGoOffline();
	void slotGoInvisible();
	void slotGoAway();
	void slotGoBusy();

protected:
	//{
	bool addContactToMetaContact( const QString &contactId, const QString &displayName,
																KopeteMetaContact *parentContact );
	//}
private:
	void initConnections();
	void initActions();

	GaduSession*           session_;
	QPtrList<GaduCommand>  commandList_;
	ContactsMap            contactsMap_;

	KActionMenu *actionMenu_;

	QTimer  *pingTimer_;

	GaduContact         *myself_;
	Q_UINT32             userUin_;
	KopeteOnlineStatus   status_;
	QString              password_;
	QString              nick_;
};

#endif
