//////////////////////////////////////////////////////////////////////////////
// gaduprotocol.h                                                           //
//                                                                          //
// Copyright (C)  2002  Zack Rusin <zack@kde.org>                           //
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
#include "gaducommands.h"

class StatusBarIcon;
class GaduSession;
class KopeteContact;
class KAction;
class KActionMenu;
class GaduContact;
class QWidget;
class KopeteMetaContact;
class GaduPreferences;

class GaduProtocol : public KopeteProtocol
{
    Q_OBJECT
public:
    typedef QMap< uin_t, GaduContact* > ContactsMap;

    GaduProtocol( QObject *parent, const char *name , const QStringList &);
    ~GaduProtocol();

    static GaduProtocol *protocol();

    // Plugin reimplementation
    // {
    void init();
    bool unload();

    QString protocolIcon() const ;
    void Connect();
    void Disconnect();
    bool isConnected() const;
    KopeteContact* createContact( KopeteMetaContact *parent,
                                          const QString &serializedData );
    void setAway();
    void setAvailable();
    bool isAway() const ;

    AddContactPage *createAddContactWidget( QWidget *parent );
    bool canSendOffline() const { return true; }
    KopeteContact *myself() const;
    void deserialize( KopeteMetaContact *metaContact, const QStringList &strList );
    QStringList addressBookFields() const;
    // }
    //!Plugin reimplementation

    void addContact( const QString& uin, const QString &nick,
                     KopeteMetaContact* parent = 0L, const QString& group = QString::null );
    void removeContact( const GaduContact *c );
public slots:
    void slotIconRightClicked( const QPoint& );
    void slotLogin();
    void slotLogoff();
    void addNotify( uin_t uin );
    void notify( uin_t *userlist, int count );
    void sendMessage( uin_t recipient, const QString& msg, int msgClass=GG_CLASS_CHAT );
    void changeStatus( int status, const QString& descr=QString::null );

    void slotGoOnline();
    void slotGoOffline();
    void slotGoInvisible();
    void slotGoAway();
    void slotGoBusy();

    void serialize( KopeteMetaContact *metaContact);

private slots:
    void settingsChanged();

protected slots:
    void error( const QString& title, const QString& message );
    void messageReceived( struct gg_event* );
    void ackReceived( struct gg_event* );
    void notify( struct gg_event* );
    void notifyDescription( struct gg_event* );
    void statusChanged( struct gg_event* );
    void pong();
    void connectionFailed( struct gg_event* );
    void connectionSucceed( struct gg_event* );
    void disconnect();
    void userlist( const QStringList& );
    void pingServer();

private:
    void initConnections();
    void initIcons();
    void initActions();

    static GaduProtocol* protocolStatic_;

    GaduSession*           session_;
    QPtrList<GaduCommand>  commandList_;
    ContactsMap            contactsMap_;

    StatusBarIcon *statusBarIcon_;

    GaduContact         *myself_;
    Q_UINT32             userUin_;
    Q_UINT32             status_;
    QString              password_;
    QString              nick_;

    GaduPreferences     *prefs_;

    QPixmap  onlineIcon_;
    QPixmap  awayIcon_;
    QPixmap  invisibleIcon_;
    QPixmap  busyIcon_;
    QPixmap  offlineIcon_;
    QPixmap  connectingIcon_;

    KActionMenu *actionMenu_;
    KAction     *onlineAction_;
    KAction     *busyAction_;
    KAction     *awayAction_;
    KAction     *invisibleAction_;
    KAction     *offlineAction_;

    QTimer  *pingTimer_;
};


#endif

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 * vim: set et ts=4 sts=4 sw=4:
 */
