//////////////////////////////////////////////////////////////////////////////
// gaduprotocol.h                                                           //
//                                                                          //
// Copyright (C)  2002-2003  Zack Rusin <zack@kde.org>                      //
//                                                                          //
// This program is free software; you can redistribute it and/or            //
// modify it under the terms of the GNU General Public License              //
// as published by the Free Software Foundation; either version 2           //
// of the License, or (at your option) any later version.                   //
//                                                                          //
// This program is distributed in the hope that it will be useful,          //
// but WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
// GNU General Public License for more details.                             //
//                                                                          //
// You should have received a copy of the GNU General Public License        //
// along with this program; if not, write to the Free Software              //
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA                //
// 02111-1307, USA.                                                         //
//////////////////////////////////////////////////////////////////////////////
#ifndef GADUPROTOCOL_H
#define GADUPROTOCOL_H

#include <qmap.h>
#include <qptrlist.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qpoint.h>

#include <libgadu.h>

#include "kopeteprotocol.h"
#include "kopeteonlinestatus.h"

#include "gaducommands.h"

class GaduSession;
class KopeteContact;
class KAction;
class KActionMenu;
class GaduContact;
class GaduAccount;
class QWidget;
class KopeteMetaContact;
class GaduPreferences;

class GaduProtocol : public KopeteProtocol
{
	Q_OBJECT
public:
	GaduProtocol( QObject *parent, const char *name, const QStringList &str);
	~GaduProtocol();

	static GaduProtocol *protocol();

	// Plugin reimplementation
	// {
	AddContactPage* createAddContactWidget( QWidget *parent, KopeteAccount* account );
  KopeteAccount* createNewAccount( const QString& accountId );
  EditAccountWidget *createEditAccountWidget( KopeteAccount *account, QWidget *parent );
	bool canSendOffline() const { return true; }

	virtual void deserializeContact( KopeteMetaContact *metaContact,
																	 const QMap<QString, QString> &serializedData,
																	 const QMap<QString, QString> &addressBookData );
	// }
	//!Plugin reimplementation

  KopeteOnlineStatus convertStatus( uint ) const;

private slots:
  void settingsChanged();

private:
	static GaduProtocol* protocolStatic_;

	GaduPreferences     *prefs_;

	const KopeteOnlineStatus gaduStatusOffline_;
	const KopeteOnlineStatus gaduStatusOfflineDescr_;
  const KopeteOnlineStatus gaduStatusMaybeOffline_;
	const KopeteOnlineStatus gaduStatusBusy_;
	const KopeteOnlineStatus gaduStatusBusyDescr_;
	const KopeteOnlineStatus gaduStatusInvisible_;
	const KopeteOnlineStatus gaduStatusInvisibleDescr_;
	const KopeteOnlineStatus gaduStatusAvail_;
	const KopeteOnlineStatus gaduStatusAvailDescr_;

  GaduAccount *defaultAccount_;
};


#endif
